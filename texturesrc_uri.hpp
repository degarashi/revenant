#pragma once
#include "texture_source.hpp"
#include "handle/uri.hpp"

namespace rev {
	//! URIから2Dテクスチャを読む
	/*!
		DeviceReset時:
		再度URIを参照
	*/
	class TextureSrc_URI :
		public TextureSource
	{
		private:
			HURI				_uri;
			bool				_mip;
		public:
			TextureSrc_URI(const HURI& uri, bool mip, InCompressedFmt_OP fmt);
			void onDeviceReset() override;
			DEF_DEBUGGUI_PROP
			DEF_DEBUGGUI_NAME
	};
	//! 6つの画像ファイルからCubeテクスチャを構成
	class TextureSrc_CubeURI :
		public TextureSource
	{
		private:
			HURI				_uri[6];
			bool				_mip;
		public:
			TextureSrc_CubeURI(
				const HURI& uri0, const HURI& uri1, const HURI& uri2,
				const HURI& uri3, const HURI& uri4, const HURI& uri5,
				bool mip, InCompressedFmt_OP fmt
			);
			void onDeviceReset() override;
			DEF_DEBUGGUI_NAME
	};
	using Size_Fmt = std::pair<lubee::SizeI, GLInCompressedFmt>;
	Size_Fmt LoadTextureFromBuffer(const TextureSource& tex, GLenum tflag, GLenum format, const lubee::SizeI& size, const ByteBuff& buff, bool bP2, bool bMip);
}