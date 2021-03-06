#pragma once
#include "../types.hpp"
#include "../format.hpp"
#include "buffer.hpp"
#include "lubee/src/rect.hpp"

namespace rev {
	namespace draw {
		class IQueue;
	}
	class PathBlock;
	using ByteBuff = std::vector<uint8_t>;
	class ITextureSource :
		public std::enable_shared_from_this<ITextureSource>,
		public IGLResource
	{
		public:
			virtual void dcmd_bind(draw::IQueue& q, GLuint actId) const = 0;
			virtual void dcmd_uniform(draw::IQueue& q, GLint id, int actId) const = 0;
			virtual void imm_bind(GLuint actId) const = 0;
			virtual bool isCubemap() const = 0;
			virtual GLuint getTextureId() const = 0;
			virtual GLuint getTextureFlag() const = 0;
			virtual lubee::SizeI getSize() const = 0;
			virtual InCompressedFmt_OP getFormat() const = 0;
			virtual GLenum getFaceFlag(CubeFace face=CubeFace::PositiveX) const = 0;
			//! 内容をファイルに保存 (主にデバッグ用)
			virtual void save(const PathBlock& path, MipLevel level=0, CubeFace face=CubeFace::PositiveX) const = 0;
			virtual TexBuffer readData(GLInFmt baseFormat, GLTypeFmt elem, MipLevel level, CubeFace face=CubeFace::PositiveX) const = 0;
			virtual TexBuffer readRect(GLInFmt baseFormat, GLTypeFmt elem, MipLevel level, const lubee::RectI& rect, CubeFace face=CubeFace::PositiveX) const = 0;
			MipBuffer readAllLayer(GLInFmt baseFormat, GLTypeFmt elem, CubeFace face=CubeFace::PositiveX) const;
			bool hasMipmap() const;
			virtual std::size_t getMipLevels() const = 0;
	};
}
