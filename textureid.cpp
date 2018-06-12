#include "gl_texture.hpp"
#include "gl_error.hpp"
#include "gl_if.hpp"
#include "handler.hpp"
#include "drawcmd/queue_if.hpp"

namespace rev {
	// ------------------------- TextureId -------------------------
	bool TextureId::_onDeviceReset() {
		if(_idTex == 0) {
			GL.glGenTextures(1, &_idTex);
			return true;
		}
		return false;
	}
	TextureId::~TextureId() {
		onDeviceLost();
	}
	void TextureId::onDeviceLost() {
		if(_idTex != 0) {
			GLW.getDrawHandler().postExecNoWait([buffId=getTextureId()](){
				GLuint id = buffId;
				GL.glDeleteTextures(1, &id);
			});
			_idTex = 0;
			D_GLWarn0();
		}
	}
	void TextureId::DCmd_Bind::Command(const void* p) {
		auto& self = *static_cast<const TextureId::DCmd_Bind*>(p);
		D_GLAssert(glActiveTexture, GL_TEXTURE0 + self.actId);
		D_GLAssert(glBindTexture, self.texFlag, self.idTex);
	}
	TextureId::TextureId(const GLuint idTex, const GLuint texFlag):
		_idTex(idTex),
		_texFlag(texFlag)
	{}
	GLuint TextureId::getTextureId() const {
		return _idTex;
	}
	GLuint TextureId::getTexFlag() const {
		return _texFlag;
	}
	bool TextureId::isCubemap() const {
		return _texFlag != GL_TEXTURE_2D;
	}
	void TextureId::dcmd_bind(draw::IQueue& q, const GLuint actId) const {
		q.add(
			DCmd_Bind {
				.idTex = getTextureId(),
				.texFlag = getTexFlag(),
				.actId = actId
			}
		);
		q.stockResource(shared_from_this());
	}
	void TextureId::imm_bind(const GLuint actId) const {
		GL.glActiveTexture(GL_TEXTURE0 + actId);
		GL.glBindTexture(getTexFlag(), getTextureId());
	}
	const char* TextureId::getResourceName() const noexcept {
		return "TextureId";
	}
	bool TextureId::operator == (const TextureId& t) const {
		return getTextureId() == t.getTextureId();
	}
}
