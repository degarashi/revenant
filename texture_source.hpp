#pragma once
#include "texturesrc_if.hpp"

namespace rev {
	class TextureSource :
		public ITextureSource
	{
		private:
			struct DCmd_Bind {
				GLuint		idTex,
							texFlag,
							actId;
				static void Command(const void* p);
			};
			struct DCmd_Uniform {
				GLint	unifId,
						actId;
				static void Command(const void* p);
			};
			GLuint				_idTex;
		protected:
			lubee::SizeI		_size;
			InCompressedFmt_OP	_format;	//!< 値が無効 = 不定
			TextureSource(InCompressedFmt_OP fmt, const lubee::SizeI& sz);
			bool _onDeviceReset();

		public:
			TextureSource(TextureSource&& t);
			virtual ~TextureSource();

			// -- from ITextureSource --
			void dcmd_bind(draw::IQueue& q, GLuint actId) const override;
			void dcmd_uniform(draw::IQueue& q, GLint id, int actId) const override;
			void imm_bind(GLuint actId) const override;
			GLuint getTextureId() const override;
			GLuint getTextureFlag() const override;
			lubee::SizeI getSize() const override;
			InCompressedFmt_OP getFormat() const override;
			GLenum getFaceFlag(CubeFace face=CubeFace::PositiveX) const override;
			void save(const PathBlock& path, CubeFace face=CubeFace::PositiveX) const override;
			ByteBuff readData(GLInFmt internalFmt, GLTypeFmt elem, int level=0, CubeFace face=CubeFace::PositiveX) const override;
			ByteBuff readRect(GLInFmt internalFmt, GLTypeFmt elem, const lubee::RectI& rect, CubeFace face=CubeFace::PositiveX) const override;

			// -- from IGLResource --
			void onDeviceLost() override;
			const char* getResourceName() const noexcept override;

			static void DCmd_ExportEmpty(draw::IQueue& q, GLint id, int actId);
			DEF_DEBUGGUI_PROP
			DEF_DEBUGGUI_SUMMARY
	};
	struct TextureLoadResult {
		lubee::SizeI		size;
		GLInCompressedFmt	format;
		std::size_t			miplevel;
	};
	TextureLoadResult LoadPixelsFromBuffer(GLenum tflag, GLenum format, const lubee::SizeI& size, const ByteBuff& buff, bool bP2, bool bMip);
}
