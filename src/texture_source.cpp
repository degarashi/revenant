#include "texture_source.hpp"
#include "gl_error.hpp"
#include "gl_if.hpp"
#include "drawcmd/queue_if.hpp"
#include "sdl/surface.hpp"
#include "sdl/rw.hpp"
#include "gl_framebuffer_tmp.hpp"
#include "gl_resource.hpp"
#include "handler.hpp"

namespace rev {
	// --------------- TextureSource::DCmd_Bind ---------------
	void TextureSource::DCmd_Bind::Command(const void* p) {
		auto& self = *static_cast<const DCmd_Bind*>(p);
		D_GLAssert(glActiveTexture, GL_TEXTURE0 + self.actId);
		D_GLAssert(glBindTexture, self.texFlag, self.idTex);
	}
	// --------------- TextureSource::DCmd_Uniform ---------------
	void TextureSource::DCmd_Uniform::Command(const void* p) {
		auto& self = *static_cast<const DCmd_Uniform*>(p);
		GL.glUniform1i(self.unifId, self.actId);
	}

	// --------------- TextureSource ---------------
	TextureSource::TextureSource(const InCompressedFmt_OP fmt, const lubee::SizeI& sz):
		_idTex(0),
		_size(sz),
		_format(fmt)
	{}
	bool TextureSource::_onDeviceReset() {
		if(_idTex == 0) {
			GL.glGenTextures(1, &_idTex);
			return true;
		}
		return false;
	}
	TextureSource::TextureSource(TextureSource&& t):
		_idTex(t._idTex),
		_size(t._size),
		_format(t._format)
	{
		t._idTex = 0;
	}
	TextureSource::~TextureSource() {
		onDeviceLost();
	}
	void TextureSource::_setSizeAndFormat(const lubee::SizeI size, const InCompressedFmt_OP fmt) {
		_size = size;
		_format = fmt;
	}
	void TextureSource::dcmd_bind(draw::IQueue& q, const GLuint actId) const {
		q.add(
			DCmd_Bind {
				.idTex = getTextureId(),
				.texFlag = getTextureFlag(),
				.actId = actId
			}
		);
		q.stockResource(shared_from_this());
	}
	void TextureSource::dcmd_uniform(draw::IQueue& q, const GLint id, const int actId) const {
		q.add(
			DCmd_Uniform{
				.unifId = id,
				.actId = actId
			}
		);
	}
	void TextureSource::imm_bind(const GLuint actId) const {
		GL.glActiveTexture(GL_TEXTURE0 + actId);
		GL.glBindTexture(getTextureFlag(), getTextureId());
	}
	GLuint TextureSource::getTextureId() const {
		return _idTex;
	}
	GLuint TextureSource::getTextureFlag() const {
		if(isCubemap())
			return GL_TEXTURE_CUBE_MAP;
		return GL_TEXTURE_2D;
	}
	lubee::SizeI TextureSource::getSize() const {
		return _size;
	}
	InCompressedFmt_OP TextureSource::getFormat() const {
		return _format;
	}
	GLenum TextureSource::getFaceFlag(const CubeFace face) const {
		if(isCubemap())
			return GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<int>(face) - static_cast<int>(CubeFace::PositiveX);
		return GL_TEXTURE_2D;
	}
	void TextureSource::save(const PathBlock& path, const MipLevel level, const CubeFace face) const {
		const auto buff = readData(GL_BGRA, GL_UNSIGNED_BYTE, level, face);
		const auto sfc = rev::Surface::Create(buff.pixels, sizeof(uint32_t)*buff.size.width, buff.size.width, buff.size.height, SDL_PIXELFORMAT_ARGB8888);
		const auto hRW = mgr_rw.fromFile(path, Access::Write);
		sfc->saveAsPNG(hRW);
	}
	TexBuffer TextureSource::readData(const GLInFmt baseFormat, const GLTypeFmt elem, const MipLevel level, const CubeFace face) const {
		auto size = getSize();
		size.width = std::max<int32_t>(1, size.width >> level);
		size.height = std::max<int32_t>(1, size.height >> level);

		const size_t sz = size.width * size.height * (*GLFormat::QueryByteSize(baseFormat, elem));
		ByteBuff buff(sz);
		GL.glPixelStorei(GL_PACK_ALIGNMENT, 1);
		#ifndef USE_OPENGLES2
		{
			imm_bind(0);
			GL.glGetTexImage(getFaceFlag(face), level, baseFormat, elem, buff.data());
		}
		#elif
		{
			// OpenGL ES2ではglGetTexImageが実装されていないのでFramebufferにセットしてglReadPixelsで取得
			// ミップマップのレベル指定は無効になる
			GLint id;
			GL.glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &id);
			GLFBufferTmp& tmp = mgr_gl.getTmpFramebuffer();
			tmp.use_begin();
			if(isCubemap())
				tmp.attachCubeTexture(GLFBuffer::Att::Color0, getTextureId(), getFaceFlag(face), level);
			else
				tmp.attachTexture(GLFBuffer::Att::Color0, getTextureId(), level);
			GL.glReadPixels(0, 0, size.width, size.height, baseFormat, elem, buff.data());
			tmp.attachTexture(GLFBuffer::Att::Color0, 0, 0);
			GL.glBindFramebuffer(GL_READ_FRAMEBUFFER_BINDING, id);
		}
		#endif
		TexBuffer ret(baseFormat, elem);
		ret.pixels = std::move(buff);
		ret.size = size;
		return ret;
	}
	TexBuffer TextureSource::readRect(const GLInFmt baseFormat, const GLTypeFmt elem, const MipLevel level, const lubee::RectI& rect, const CubeFace face) const {
		const size_t sz = rect.width() * rect.height() * (*GLFormat::QueryByteSize(baseFormat, elem));

		GLint id;
		GLenum flag;
		#ifndef USE_OPENGLES2
			flag = GL_READ_FRAMEBUFFER_BINDING;
		#elif
			flag = GL_FRAMEBUFFER_BINDING;
		#endif
		GL.glGetIntegerv(flag, &id);

		GLFBufferTmp& tmp = mgr_gl.getTmpFramebuffer();
		ByteBuff buff(sz);
		tmp.use_begin();
		if(isCubemap())
			tmp.attachCubeTexture(GLFBuffer::Att::Color0, getTextureId(), getFaceFlag(face), level);
		else
			tmp.attachTexture(GLFBuffer::Att::Color0, getTextureId(), level);
		GL.glReadPixels(rect.x0, rect.y0, rect.width(), rect.height(), baseFormat, elem, buff.data());
		tmp.attachTexture(GLFBuffer::Att::Color0, 0, 0);

		GL.glBindFramebuffer(flag, id);
		TexBuffer ret(baseFormat, elem);
		ret.pixels = std::move(buff);
		ret.size = {rect.width(), rect.height()};
		return ret;
	}
	void TextureSource::onDeviceLost() {
		if(_idTex != 0) {
			GLW.getDrawHandler().postExecNoWait([buffId=getTextureId()](){
				GLuint id = buffId;
				GL.glDeleteTextures(1, &id);
			});
			_idTex = 0;
			D_GLWarn0();
		}
	}
	const char* TextureSource::getResourceName() const noexcept {
		return "TextureSource";
	}
	void TextureSource::DCmd_ExportEmpty(draw::IQueue& q, const GLint id, const int actId) {
		const auto bind = [&q, actId](const GLuint flag) {
			q.add(
				DCmd_Bind{
					.idTex = 0,
					.texFlag = flag,
					.actId = GLuint(actId)
				}
			);
		};
		bind(GL_TEXTURE_2D);
		bind(GL_TEXTURE_CUBE_MAP);
		q.add(
			DCmd_Uniform {
				.unifId = id,
				.actId = actId
			}
		);
	}
}
