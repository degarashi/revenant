#pragma once
#include "texture_source.hpp"
#include "texture_filter.hpp"
#include "handle/opengl.hpp"

namespace rev {
	// TextureSource + Filter
	class GLTexture :
		public ITextureSource
	{
		private:
			HTexSrcC		_src;
			HTexF			_filter;
		public:
			GLTexture(const HTexSrcC& src, const HTexF& filter);

			// -- from ITextureSource --
			void dcmd_bind(draw::IQueue& q, GLuint actId) const override;
			void dcmd_uniform(draw::IQueue& q, GLint id, int actId) const override;
			void imm_bind(GLuint actId) const override;
			bool isCubemap() const override;
			GLuint getTextureId() const override;
			GLuint getTextureFlag() const override;
			lubee::SizeI getSize() const override;
			InCompressedFmt_OP getFormat() const override;
			GLenum getFaceFlag(CubeFace face=CubeFace::PositiveX) const override;
			void save(const PathBlock& path, CubeFace face=CubeFace::PositiveX) const override;
			ByteBuff readData(GLInFmt internalFmt, GLTypeFmt elem, int level=0, CubeFace face=CubeFace::PositiveX) const override;
			ByteBuff readRect(GLInFmt internalFmt, GLTypeFmt elem, const lubee::RectI& rect, CubeFace face=CubeFace::PositiveX) const override;
			std::size_t getMipLevels() const override;

			const HTexSrcC& texture() const noexcept;
			HTexF& filter() noexcept;
			const HTexF& filter() const noexcept;
	};
}
