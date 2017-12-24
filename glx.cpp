#include "glx.hpp"
#include "gl_error.hpp"
#include "gl_program.hpp"
#include "glx_if.hpp"
#include "uniform_pool.hpp"
#include <boost/format.hpp>

namespace rev {
	GLEffect::EC_FileNotFound::EC_FileNotFound(const std::string& fPath):
		EC_Base((boost::format("file path: \"%1%\" was not found.") % fPath).str())
	{}
	// -------------------------- UniformMap --------------------------
	draw::TokenBuffer* UniformMap::makeTokenBuffer(const GLint id) {
		const auto itr = _map.find(id);
		if(itr == _map.end()) {
			return _map[id] = unif_pool.allocate();
		}
		return itr->second;
	}
	void UniformMap::clear() {
		auto& pool = unif_pool;
		for(auto& p : _map)
			pool.destroy(p.second);
		_map.clear();
	}
	void UniformMap::copyFrom(const UniformMap& other) {
		for(auto& o : other._map) {
			auto* buff = makeTokenBuffer(o.first);
			o.second->clone(*buff);
		}
	}
	void UniformMap::moveTo(draw::TokenML& ml) {
		auto& pool = unif_pool;
		for(auto& u : _map) {
			u.second->takeout(ml);
			pool.destroy(u.second);
		}
		_map.clear();
	}
	bool UniformMap::empty() const noexcept {
		return _map.empty();
	}
	// ----------------- GLEffect -----------------
	GLEffect::GLEffect(const std::string& name) {
		_blockSet = parse::LoadGLXStructSet(name);
		std::vector<parse::TPStruct*> tpV;
		for(auto& blk : *_blockSet) {
			auto& b = *blk;
			for(auto& tp : b.tpL)
				tpV.emplace_back(&tp);
		}
		try {
			// Tech/Passを順に実行形式へ変換
			// (一緒にTech/Pass名リストを構築)
			const int nI = tpV.size();
			_techName.resize(nI);
			for(int techId=0 ; techId<nI ; techId++) {
				auto& nmm = _techName[techId];
				auto& tpTech = *tpV.at(techId);
				// Pass毎に処理
				int nJ = tpTech.tpL.size();
				nmm.resize(nJ+1);
				nmm[0] = tpTech.name;
				for(int passId=0 ; passId<nJ ; passId++) {
					nmm[passId+1] = tpTech.tpL.at(passId).get().name;
					GL16Id tpid{uint8_t(techId), uint8_t(passId)};
					auto res = _techMap.insert(std::make_pair(tpid, TPStructR(_blockSet, tpTech, tpTech.tpL.at(passId).get())));
					// テクスチャインデックスリスト作成
					TPStructR& tpr = res.first->second;
					GLuint pid = tpr.getProgram()->getProgramId();
					GLint nUnif;
					GL.glGetProgramiv(pid, GL_ACTIVE_UNIFORMS, &nUnif);

					// Sampler2D変数が見つかった順にテクスチャIdを割り振る
					GLint curI = 0;
					TexIndex& texIndex = _texMap[tpid];
					for(GLint i=0 ; i<nUnif ; i++) {
						GLsizei len;
						int size;
						GLenum typ;
						GLchar cbuff[0x100];	// GLSL変数名の最大がよくわからない (ので、数は適当)

						D_GLAssert(glGetActiveUniform, pid, i, sizeof(cbuff), &len, &size, &typ, cbuff);
						auto opInfo = GLFormat::QueryGLSLInfo(typ);
						if(opInfo->type == GLSLType::TextureT) {
							// GetActiveUniformでのインデックスとGetUniformLocationIdは異なる場合があるので・・
							GLint id = D_GLAssert(glGetUniformLocation, pid, cbuff);
							Assert0(id>=0);
							texIndex.insert(std::make_pair(id, curI++));
						}
					}
				}
			}
		} catch(const std::exception& e) {
			std::cout << "GLEffect exception: " << e.what() << std::endl;
			throw;
		}
		D_GLAssert0();

		_setConstantUniformList(&GlxId::GetUnifList());
		_setConstantTechPassList(&GlxId::GetTechList());
	}
}
