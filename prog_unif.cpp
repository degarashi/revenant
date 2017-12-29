#include "prog_unif.hpp"
#include "gl_program.hpp"
#include "drawtoken/texture.hpp"

namespace rev {
	draw::TokenBuffer& Prog_Unif::_makeUniformTokenBuffer(const GLint id) {
		return *uniValue.makeTokenBuffer(id);
	}
	GLint_OP Prog_Unif::getUniformId(const Name& name) const {
		D_Assert(program, "shader program handle is invalid");
		return program->getUniformId(name);
	}
	// Uniform設定は一旦_unifMapに蓄積した後、出力
	void Prog_Unif::_makeUniformToken(draw::TokenDst& dst, const GLint id, const bool* b, const int n, const bool bT) const {
		int tmp[n];
		for(int i=0 ; i<n ; i++)
			tmp[i] = static_cast<int>(b[i]);
		_makeUniformToken(dst, id, static_cast<const int*>(tmp), 1, bT);
	}
	void Prog_Unif::_makeUniformToken(draw::TokenDst& dst, const GLint id, const int* iv, const int n, bool /*bT*/) const {
		MakeUniformToken<draw::Unif_Vec<int, 1>>(dst, id, id, iv, 1, n);
	}
	void Prog_Unif::_makeUniformToken(draw::TokenDst& dst, const GLint id, const float* fv, const int n, bool /*bT*/) const {
		MakeUniformToken<draw::Unif_Vec<float, 1>>(dst, id, id, fv, 1, n);
	}
	void Prog_Unif::_makeUniformToken(draw::TokenDst& dst, const GLint id, const double* dv, const int n, const bool bT) const {
		float tmp[n];
		for(int i=0 ; i<n ; i++)
			tmp[i] = static_cast<float>(dv[i]);
		_makeUniformToken(dst, id, static_cast<const float*>(tmp), n, bT);
	}
	void Prog_Unif::_makeUniformToken(draw::TokenDst& dst, const GLint id, const HTex* hTex, const int n, bool /*bT*/) const {
		// テクスチャユニット番号を検索
		const auto& tIdx = program->getTexIndex();
		const auto itr = tIdx.find(id);
		Expect(itr != tIdx.end(), "texture index not found");
		if(itr != tIdx.end()) {
			const auto aId = itr->second;
			if(n > 1) {
				std::vector<const IGLTexture*> pTexA(n);
				for(int i=0 ; i<n ; i++)
					pTexA[i] = hTex[i].get();
				MakeUniformToken<draw::TextureA>(dst, id,
								id, hTex,
								pTexA.data(), aId, n);
				return;
			}
			D_Assert0(n==1);
			const HTex hTex2(*hTex);
			hTex2->getDrawToken(dst, id, 0, aId);
		}
	}
}
