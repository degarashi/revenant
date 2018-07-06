#include "gl_texture.hpp"

namespace rev {
	GLTexture::GLTexture(const HTexSrcC& src, const HTexF& filter):
		_src(src),
		_filter(filter)
	{}
	void GLTexture::dcmd_bind(draw::IQueue& q, const GLuint actId) const {
		_src->dcmd_bind(q, actId);
		_filter->dcmd_filter(q, _src->getTextureFlag());
	}
	void GLTexture::dcmd_uniform(draw::IQueue& q, const GLint id, const int actId) const {
		dcmd_bind(q, actId);
		_src->dcmd_uniform(q, id, actId);
	}
	void GLTexture::imm_bind(const GLuint actId) const {
		_src->imm_bind(actId);
		_filter->imm_filter(_src->getTextureFlag());
	}
	bool GLTexture::isCubemap() const {
		return _src->isCubemap();
	}
	GLuint GLTexture::getTextureId() const {
		return _src->getTextureId();
	}
	GLuint GLTexture::getTextureFlag() const {
		return _src->getTextureFlag();
	}
	lubee::SizeI GLTexture::getSize() const {
		return _src->getSize();
	}
	InCompressedFmt_OP GLTexture::getFormat() const {
		return _src->getFormat();
	}
	GLenum GLTexture::getFaceFlag(const CubeFace face) const {
		return _src->getFaceFlag(face);
	}
	void GLTexture::save(const PathBlock& path, const CubeFace face) const {
		_src->save(path, face);
	}
	ByteBuff GLTexture::readData(const GLInFmt internalFmt, const GLTypeFmt elem, const int level, const CubeFace face) const {
		return _src->readData(internalFmt, elem, level, face);
	}
	ByteBuff GLTexture::readRect(const GLInFmt internalFmt, const GLTypeFmt elem, const lubee::RectI& rect, const CubeFace face) const {
		return _src->readRect(internalFmt, elem, rect, face);
	}
	HTexF& GLTexture::filter() noexcept {
		return _filter;
	}
	const HTexF& GLTexture::filter() const noexcept {
		return _filter;
	}
	const HTexSrcC& GLTexture::texture() const noexcept {
		return _src;
	}
	std::size_t GLTexture::getMipLevels() const {
		return _src->getMipLevels();
	}
}
