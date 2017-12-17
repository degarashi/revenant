#include "glx.hpp"
#include "gl_error.hpp"
#include "gl_program.hpp"
#include "sdl_rw.hpp"
#include "glx_if.hpp"
#include <boost/format.hpp>

namespace rev {
	const ValueSettingR::VSFunc ValueSettingR::cs_func[] = {
		BOOST_PP_SEQ_FOR_EACH(PPFUNC_GLSET_FUNC, EMPTY, SEQ_GLSETTING)
	};
	const int ValueSettingR::cs_funcNArg[] = {
		BOOST_PP_SEQ_FOR_EACH(PPFUNC_GLSET_NARG, EMPTY, SEQ_GLSETTING)
	};
	const char* ValueSettingR::cs_funcName[] = {
		BOOST_PP_SEQ_FOR_EACH(PPFUNC_GLSET_NAME, EMPTY, SEQ_GLSETTING)
	};

	GLEffect::EC_FileNotFound::EC_FileNotFound(const std::string& fPath):
		EC_Base((boost::format("file path: \"%1%\" was not found.") % fPath).str())
	{}

	// -------------- ValueSettingR --------------
	ValueSettingR::ValueSettingR(const ValueSetting& s) {
		func = cs_func[s.type];
		const int nV = std::min(s.value.size(), countof(value));
		// 引数の数が合わなかったらエラー
		const int ArgLength = cs_funcNArg[s.type];
		if(ArgLength != nV)
			throw  std::runtime_error((boost::format("amount of argument(s) is not valid(func=%1%, required=%2%, actual=%3%)") % cs_funcName[s.type] % ArgLength % nV).str());
		for(int i=0 ; i<nV ; i++)
			value[i] = s.value[i];
		for(int i=nV ; i<static_cast<int>(countof(value)) ; i++)
			value[i] = boost::blank();
	}
	void ValueSettingR::action() const {
		func(*this);
	}
	bool ValueSettingR::operator == (const ValueSettingR& s) const {
		for(int i=0 ; i<static_cast<int>(countof(value)) ; i++)
			if(!(value[i] == s.value[i]))
				return false;
		return func == s.func;
	}

	// -------------- BoolSettingR --------------
	const BoolSettingR::VBFunc BoolSettingR::cs_func[] = {
		&IGL::glEnable, &IGL::glDisable
	};
	BoolSettingR::BoolSettingR(const BoolSetting& s) {
		func = cs_func[(s.value) ? 0 : 1];
		flag = s.type;
	}
	void BoolSettingR::action() const {
		(GL.*func)(flag);
	}
	bool BoolSettingR::operator == (const BoolSettingR& s) const {
		return flag==s.flag && func==s.func;
	}

	namespace {
		const std::string cs_rtname[] = {
			"effect"
		};
		BlockSet LoadGLXStructSet(const std::string& name) {
			BlockSet bs;
			std::unordered_set<std::string> loaded,
											inclset{name};
			// inclset = まだ読み込んでないファイル名
			while(!inclset.empty()) {
				// inclsetから1つ取り出して読み込み
				auto itr = inclset.begin();
				auto hdl = mgr_block.loadResourceApp<GLXStruct>(
								UserURI(*itr),
								[](auto& uri, auto&& mk){
									auto s = mgr_rw.fromURI(*uri.uri, Access::Read);
									mk();
									ParseGlx(*mk.pointer, s->readAllAsString());
								}
							).first;
				loaded.emplace(*itr);
				inclset.erase(itr);
				// まだ読み込んでないファイルをinclsetに加える
				for(auto& inc : hdl->incl) {
					if(loaded.count(inc) == 0)
						inclset.emplace(inc);
				}
				bs.emplace(std::move(hdl));
			}
			return bs;
		}
	}
	// ----------------- FxBlock -----------------
	FxBlock::FxBlock():
		ResMgrApp(cs_rtname)
	{}
	// ----------------- BlockSet -----------------
	template <class RET, class GETM>
	auto FindBlock(const BlockSet& bs, GETM getM, const std::string& s) -> spi::Optional<const RET&> {
		for(auto& b : bs) {
			const auto& m = getM(*b).get();
			auto itr = m.find(s);
			if(itr != m.cend())
				return itr->second;
		}
		return spi::none;
	}
	spi::Optional<const AttrStruct&> BlockSet::findAttribute(const std::string& s) const {
		return FindBlock<AttrStruct>(*this, [](auto&& str) { return std::cref(str.atM); }, s);
	}
	spi::Optional<const ConstStruct&> BlockSet::findConst(const std::string& s) const {
		return FindBlock<ConstStruct>(*this, [](auto&& str) { return std::cref(str.csM); }, s);
	}
	spi::Optional<const UnifStruct&> BlockSet::findUniform(const std::string& s) const {
		return FindBlock<UnifStruct>(*this, [](auto&& str) { return std::cref(str.uniM); }, s);
	}
	spi::Optional<const VaryStruct&> BlockSet::findVarying(const std::string& s) const {
		return FindBlock<VaryStruct>(*this, [](auto&& str) { return std::cref(str.varM); }, s);
	}
	spi::Optional<const ShStruct&> BlockSet::findShader(const std::string& s) const {
		return FindBlock<ShStruct>(*this, [](auto&& str) { return std::cref(str.shM); }, s);
	}
	spi::Optional<const TPStruct&> BlockSet::findTechPass(const std::string& s) const {
		for(auto& b : *this) {
			auto& tpL = b->tpL;
			auto itr = std::find_if(tpL.cbegin(), tpL.cend(), [&](auto& tp){ return tp.name == s; });
			if(itr != tpL.cend())
				return *itr;
		}
		return spi::none;
	}
	spi::Optional<const CodeStruct&> BlockSet::findCode(const std::string& s) const {
		return FindBlock<CodeStruct>(*this, [](auto&& str) { return std::cref(str.codeM); }, s);
	}

	// IEffect::GlxId IEffect::s_myId;
	draw::TokenBuffer* MakeUniformTokenBuffer(UniMap& um, UnifPool& pool, GLint id) {
		auto itr = um.find(id);
		if(itr == um.end()) {
			return um[id] = pool.allocate();
		}
		return itr->second;
	}

	// ----------------- GLEffect -----------------
	GLEffect::GLEffect(const std::string& name) {
		_blockSet = LoadGLXStructSet(name);
		std::vector<TPStruct*> tpV;
		for(auto& blk : _blockSet) {
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
