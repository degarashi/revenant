#pragma once
#include "texture_source.hpp"
#include "abstbuffer.hpp"

namespace rev {
	class TextureSrc_Mem :
		public TextureSource
	{
		protected:
			struct Cache {
				ByteBuff	buff;		//!< DeviceLost時用のバッファ
				GLTypeFmt	format;		//!< _buffに格納されているデータの形式(Type)

				Cache(GLTypeFmt fmt);
			};
			using Cache_Op = spi::Optional<Cache>;
			Cache_Op		_cache;

			virtual Cache _backupBuffer() const = 0;
			virtual void _restoreBuffer(const Cache_Op& c) = 0;

		private:
			bool			_restore,
							_mip;
		protected:
			bool _restoreFlag() const noexcept;
			bool _mipFlag() const noexcept;
		public:
			TextureSrc_Mem(GLInSizedFmt fmt, const lubee::SizeI& sz, bool mip, bool bRestore);
			// -- from IGLResource --
			void onDeviceReset() override;
			void onDeviceLost() override;
	};
	//! ユーザー定義の空テクスチャ
	/*!
		DeviceLost時の復元は任意
		内部バッファはDeviceLost用であり、DeviceがActiveな時はnone
		フォーマット変換は全てOpenGLにさせる
	*/
	class TextureSrc_Mem2D :
		public TextureSrc_Mem
	{
		private:
			Cache _backupBuffer() const override;
			void _restoreBuffer(const Cache_Op& c) override;

		public:
			using TextureSrc_Mem::TextureSrc_Mem;
			//! テクスチャ全部書き換え = バッファも置き換え
			/*
				\param[in] fmt テクスチャのフォーマット
				\param[in] srcFmt 入力フォーマット(Type)
			*/
			void writeData(AB_Byte buff, GLTypeFmt srcFmt);
			//! 部分的に書き込み
			/*!
				現状ではMipmap有りでの書き込みには非対応
				\param[in] rect 書き込み先座標
				\param[in] srcFmt 入力フォーマット(Type)
			*/
			void writeRect(AB_Byte buff, const lubee::RectI& rect, GLTypeFmt srcFmt);

			bool isCubemap() const override;
			std::size_t getMipLevels() const override;

			DEF_DEBUGGUI_NAME
	};
	class TextureSrc_MemCube :
		public TextureSrc_Mem
	{
		private:
			Cache _backupBuffer() const override;
			void _restoreBuffer(const Cache_Op& c) override;

			template <class CB>
			void Iter(CB&& cb) const;
		public:
			using TextureSrc_Mem::TextureSrc_Mem;
			//! テクスチャ全部書き換え = バッファも置き換え
			/*!
				\param[in] fmt テクスチャのフォーマット
				\param[in] srcFmt 入力フォーマット(Type)
				\param[in] face Cubemapにおける面
			*/
			void writeData(AB_Byte buff, GLTypeFmt srcFmt, CubeFace face);
			//! 部分的に書き込み
			/*!
				現状ではMipmap有りでの書き込みには非対応
				\param[in] rect 書き込み先座標
				\param[in] srcFmt 入力フォーマット(Type)
				\param[in] face Cubemapにおける面
			*/
			void writeRect(AB_Byte buff, const lubee::RectI& rect, GLTypeFmt srcFmt, CubeFace face);

			bool isCubemap() const override;
			std::size_t getMipLevels() const override;

			DEF_DEBUGGUI_NAME
	};
}
