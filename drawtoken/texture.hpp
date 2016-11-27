#pragma once
#include "drawtoken_t.hpp"
#include "../gl_texture.hpp"

namespace rev {
	namespace draw {
		// 連番テクスチャはUniformId + Indexとして設定
		class Texture : public IGLTexture, public Uniform<Texture> {
			private:
				HTex	_hTex;
			public:
				Texture(const HTex& hTex, GLint id, int index, int baseActId,  const IGLTexture& t);
				virtual ~Texture();
				void exec() override;
		};
		class TextureA : public Uniform<TextureA> {
			private:
				using TexA = std::vector<Texture>;
				TexA	_texA;
			public:
				TextureA(GLint id, const HTex* hTex, const IGLTexture** tp, int baseActId, int nTex);
				void exec() override;
		};
	}
}
