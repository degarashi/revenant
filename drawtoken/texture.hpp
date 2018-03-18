#pragma once
#include "drawtoken_t.hpp"
#include "../gl_texture.hpp"
#include "../handle/opengl.hpp"

namespace rev {
	namespace draw {
		// 連番テクスチャはUniformId + Indexとして設定
		class Texture : public IGLTexture, public Uniform<Texture> {
			private:
				HTex			_hTex;
				mutable int		_actId;
			public:
				Texture(const HTex& hTex);
				void exportToken(TokenDst& dst, GLint id, int activeTexId) const override;
				void setIds(GLint id, int activeTexId) const;
				virtual ~Texture();
				void exec() override;
		};
		class TextureA : public Uniform<TextureA> {
			private:
				using TexA = std::vector<Texture>;
				TexA	_texA;
			public:
				TextureA(const HTex* hTex, int nTex);
				void exportToken(TokenDst& dst, GLint id, int activeTexId) const override;
				void exec() override;
		};
	}
}
