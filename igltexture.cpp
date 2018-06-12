#include "gl_texture.hpp"
#include "gl_error.hpp"
#include "gl_if.hpp"
#include "gl_framebuffer.hpp"
#include "gl_resource.hpp"
#include "drawcmd/queue_if.hpp"
#include "sdl_surface.hpp"
#include "sdl_rw.hpp"
#include "handler.hpp"

namespace rev {
	// --------------- IGLTexture::DCmd_Uniform ---------------
	void IGLTexture::DCmd_Uniform::Command(const void* p) {
		auto& self = *static_cast<const DCmd_Uniform*>(p);
		GL.glUniform1i(self.unifId, self.actId);
	}
	// --------------- IGLTexture::DCmd_ExportEmpty ---------------
	void IGLTexture::DCmd_ExportEmpty(draw::IQueue& q, const GLint id, const int actId) {
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
	// --------------- IGLTexture::DCmd_Bind ---------------
	void IGLTexture::DCmd_Bind::Command(const void* p) {
		auto& self = *static_cast<const IGLTexture::DCmd_Bind*>(p);
		D_GLAssert(glActiveTexture, GL_TEXTURE0 + self.actId);
		D_GLAssert(glBindTexture, self.texFlag, self.idTex);
	}

	// --------------- IGLTexture ---------------
	bool IGLTexture::_onDeviceReset() {
		if(_idTex == 0) {
			GL.glGenTextures(1, &_idTex);
			return true;
		}
		return false;
	}
	IGLTexture::IGLTexture(const MipState miplevel, const InCompressedFmt_OP fmt, const lubee::SizeI& sz, const bool bCube):
		_idTex(0),
		_texFlag(bCube ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D),
		_faceFlag(bCube ? GL_TEXTURE_CUBE_MAP_POSITIVE_X : GL_TEXTURE_2D),
		_filter(
			0,
			0,
			WrapState::ClampToEdge,
			WrapState::ClampToEdge,
			miplevel,
			0
		),
		_size(sz),
		_format(fmt)
	{
		Assert0(!bCube || (_size.width==_size.height));
	}
	IGLTexture::~IGLTexture() {
		onDeviceLost();
	}
	const lubee::SizeI& IGLTexture::getSize() const {
		return _size;
	}
	const InCompressedFmt_OP& IGLTexture::getFormat() const {
		return _format;
	}
	GLenum IGLTexture::getFaceFlag(const CubeFace face) const {
		if(isCubemap())
			return _faceFlag + static_cast<int>(face) - static_cast<int>(CubeFace::PositiveX);
		return _faceFlag;
	}
	void IGLTexture::save(const PathBlock& path, const CubeFace face) {
		auto buff = readData(GL_BGRA, GL_UNSIGNED_BYTE, 0, face);
		auto sfc = rev::Surface::Create(buff, sizeof(uint32_t)*_size.width, _size.width, _size.height, SDL_PIXELFORMAT_ARGB8888);
		// OpenGLテクスチャは左下が原点なので…
		auto sfcVf = sfc->flipVertical();
		auto hRW = mgr_rw.fromFile(path, Access::Write);
		sfcVf->saveAsPNG(hRW);
	}
	ByteBuff IGLTexture::readData(const GLInFmt internalFmt, const GLTypeFmt elem, const int level, const CubeFace face) const {
		auto size = getSize();
		const size_t sz = size.width * size.height * GLFormat::QueryByteSize(internalFmt, elem);
		ByteBuff buff(sz);
		#ifndef USE_OPENGLES2
		{
			imm_bind(0);
			GL.glGetTexImage(getFaceFlag(face), level, internalFmt, elem, buff.data());
		}
		#elif
		{
			//	OpenGL ES2ではglGetTexImageが実装されていないのでFramebufferにセットしてglReadPixelsで取得
			GLint id;
			GL.glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &id);
			GLFBufferTmp& tmp = mgr_gl.getTmpFramebuffer();
			imm_bind(0);
			if(isCubemap())
				tmp.attachCubeTexture(GLFBuffer::Att::COLOR0, getTextureId(), getFaceFlag(face));
			else
				tmp.attachTexture(GLFBuffer::Att::COLOR0, getTextureId());
			GL.glReadPixels(0, 0, size.width, size.height, internalFmt, elem, buff.data());
			tmp.attachTexture(GLFBuffer::Att::COLOR0, 0);
			GL.glBindFramebuffer(GL_READ_FRAMEBUFFER_BINDING, id);
		}
		#endif
		return buff;
	}
	ByteBuff IGLTexture::readRect(const GLInFmt internalFmt, const GLTypeFmt elem, const lubee::RectI& rect, const CubeFace face) const {
		const size_t sz = rect.width() * rect.height() * GLFormat::QueryByteSize(internalFmt, elem);

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
			tmp.attachCubeTexture(GLFBuffer::Att::COLOR0, getTextureId(), getFaceFlag(face));
		else
			tmp.attachTexture(GLFBuffer::Att::COLOR0, getTextureId());
		GL.glReadPixels(rect.x0, rect.y0, rect.width(), rect.height(), internalFmt, elem, buff.data());
		tmp.attachTexture(GLFBuffer::Att::COLOR0, 0);

		GL.glBindFramebuffer(flag, id);
		return buff;
	}

	void IGLTexture::dcmd_bind(draw::IQueue& q, const GLuint actId) const {
		q.add(
			DCmd_Bind {
				.idTex = getTextureId(),
				.texFlag = getTextureFlag(),
				.actId = actId
			}
		);
		q.stockResource(shared_from_this());
	}
	void IGLTexture::dcmd_uniform(draw::IQueue& q, const GLint id, const int actId) const {
		dcmd_bind(q, actId);
		_filter.dcmd_filter(q, getTextureFlag());
		q.add(
			DCmd_Uniform{
				.unifId = id,
				.actId = actId
			}
		);
		q.stockResource(shared_from_this());
	}
	void IGLTexture::imm_bind(const GLuint actId) const {
		GL.glActiveTexture(GL_TEXTURE0 + actId);
		GL.glBindTexture(getTextureFlag(), getTextureId());
	}
	bool IGLTexture::isCubemap() const {
		return _texFlag != GL_TEXTURE_2D;
	}
	GLuint IGLTexture::getTextureId() const {
		return _idTex;
	}
	GLuint IGLTexture::getTextureFlag() const {
		return _texFlag;
	}
	TextureFilter& IGLTexture::filter() {
		return _filter;
	}
	const TextureFilter& IGLTexture::filter() const {
		return _filter;
	}
	void IGLTexture::onDeviceLost() {
		if(_idTex != 0) {
			GLW.getDrawHandler().postExecNoWait([buffId=getTextureId()](){
				GLuint id = buffId;
				GL.glDeleteTextures(1, &id);
			});
			_idTex = 0;
			D_GLWarn0();
		}
	}
	const char* IGLTexture::getResourceName() const noexcept {
		return "IGLTexture";
	}
}
