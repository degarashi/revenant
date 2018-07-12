#pragma once
#include "texture_source.hpp"
#include "abstbuffer.hpp"

namespace rev {
	//! ユーザー定義の空テクスチャ
	/*!
		DeviceLost時の復元は任意
		内部バッファはDeviceLost用であり、DeviceがActiveな時はnone
		フォーマット変換は全てOpenGLにさせる
		書き込み不可の時は最初の一度だけ書き込める
	*/
	class TextureSrc_Mem :
		public TextureSource
	{
		private:
			using Buff_OP = spi::Optional<ByteBuff>;
			using Format_OP = spi::Optional<GLTypeFmt>;
			Buff_OP			_buff;			//!< DeviceLost時用のバッファ
			Format_OP		_typeFormat;	//!< _buffに格納されているデータの形式(Type)

			// bool		_bStream;		//!< 頻繁に書き換えられるか(の、ヒント)
			bool		_bRestore;
			//! テクスチャフォーマットから必要なサイズを計算してバッファを用意する
			const GLFormatDesc& _prepareBuffer();
		public:
			TextureSrc_Mem(bool bCube, GLInSizedFmt fmt, const lubee::SizeI& sz, bool bStream, bool bRestore);
			//! テクスチャ全部書き換え = バッファも置き換え
			/*! \param[in] fmt テクスチャのフォーマット
				\param[in] srcFmt 入力フォーマット(Type)
				\param[in] bRestore trueなら内部バッファにコピーを持っておいてDeviceLostに備える
				\param[in] face Cubemapにおける面 */
			void writeData(AB_Byte buff, GLTypeFmt srcFmt, CubeFace face=CubeFace::PositiveX);
			//! 部分的に書き込み
			/*! \param[in] ofsX 書き込み先オフセット X
				\param[in] ofsY 書き込み先オフセット Y
				\param[in] srcFmt 入力フォーマット(Type)
				\param[in] face Cubemapにおける面 */
			void writeRect(AB_Byte buff, const lubee::RectI& rect, GLTypeFmt srcFmt, CubeFace face=CubeFace::PositiveX);
			bool hasMipmap() const override;
			std::size_t getMipLevels() const override;

			// -- from IGLResource --
			void onDeviceReset() override;
			void onDeviceLost() override;

			DEF_DEBUGGUI_NAME
	};
}
