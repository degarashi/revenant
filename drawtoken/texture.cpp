#include "texture.hpp"
#include "../gl_if.hpp"
#include "../videoparam.hpp"

namespace rev::draw {
	// ------------------------- draw::Texture -------------------------
	namespace {
		const GLenum cs_wrap[WrapState::_Num] = {
			GL_CLAMP_TO_EDGE,
			GL_CLAMP_TO_BORDER,
			GL_MIRRORED_REPEAT,
			GL_REPEAT,
			GL_MIRROR_CLAMP_TO_EDGE
		};
	}
	// --------------- Texture ---------------
	Texture::Texture(const HTexC& hTex):
		IGLTexture(*hTex),
		_hTex(hTex),
		_actId(-1)
	{}
	Texture::~Texture() {
		// IGLTextureのdtorでリソースを開放されないように0にセットしておく
		_idTex = 0;
	}
	void Texture::setIds(const GLint id, const int activeTexId) const {
		idUnif = id;
		_actId = activeTexId;
	}
	void Texture::exportToken(TokenDst& dst, const GLint id, const int activeTexId) const {
		_actId = activeTexId;
		Uniform::exportToken(dst, id);
	}
	void Texture::exec() {
		setActiveId(_actId);
		// 最後にBindは解除しない
		use_begin();
		{
			if(tls_videoParam.get().bAnisotropic) {
				// setAnisotropic
				GLfloat aMax;
				GL.glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aMax);
				GL.glTexParameteri(_texFlag, GL_TEXTURE_MAX_ANISOTROPY_EXT, std::max(aMax*_coeff, 1.f));
			}
			// setUVWrap
			GL.glTexParameteri(_texFlag, GL_TEXTURE_WRAP_S, cs_wrap[_wrapS]);
			GL.glTexParameteri(_texFlag, GL_TEXTURE_WRAP_T, cs_wrap[_wrapT]);
			// setFilter
			GL.glTexParameteri(_texFlag, GL_TEXTURE_MAG_FILTER, cs_Filter[0][_iLinearMag]);
			GL.glTexParameteri(_texFlag, GL_TEXTURE_MIN_FILTER, cs_Filter[_mipLevel][_iLinearMin]);
		}
		GL.glUniform1i(idUnif, _actId);
	}
	bool Texture::isArray() const noexcept {
		return false;
	}

	// --------------- TextureA ---------------
	TextureA::TextureA(const std::vector<HTexC>& t):
		TextureA(t.cbegin(), t.cend())
	{}
	void TextureA::exportToken(TokenDst& dst, const GLint id, const int activeTexId) const {
		const auto nT = _texA.size();
		for(std::size_t i=0 ; i<nT ; i++) {
			_texA[i].setIds(id+i, activeTexId+i);
		}
		Uniform::exportToken(dst, id);
	}
	void TextureA::exec() {
		for(auto& p : _texA)
			p.exec();
	}
	bool TextureA::isArray() const noexcept {
		return true;
	}
}
