#pragma once
#include "source.hpp"
#include "../../abstbuffer.hpp"

namespace rev {
	class TextureSrc_Mem :
		public TextureSource
	{
		protected:
			virtual void _backupBuffer() const = 0;
			virtual void _restoreBuffer() = 0;
			bool _restoreFlag() const noexcept;

		private:
			bool			_restore,
							_mipEnable;
			bool _mipEnabled() const noexcept;
		protected:
			bool			_mipAuto;
			bool _manualMip() const noexcept;
			bool _autoMip() const noexcept;
		public:
			TextureSrc_Mem(GLInSizedFmt fmt, const lubee::SizeI& sz, bool mipEnable, bool bRestore);
			std::size_t getMipLevels() const override;
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
			using Cache_Op = spi::Optional<MipBuffer>;
			mutable Cache_Op	_cache;
			void _backupBuffer() const override;
			void _restoreBuffer() override;
			void _writeData(AB_Byte buff, GLTypeFmt elem, MipLevel level, bool mipAuto);
			void _writeRect(AB_Byte buff, const lubee::RectI& rect, GLTypeFmt elem, MipLevel level, bool mipAuto);

		public:
			using TextureSrc_Mem::TextureSrc_Mem;
			//! テクスチャ全部書き換え = バッファも置き換え
			/*
				\param[in] fmt テクスチャのフォーマット
				\param[in] elem 入力フォーマット(Type)
			*/
			void writeData(AB_Byte buff, GLTypeFmt elem);
			void writeData(AB_Byte buff, GLTypeFmt elem, MipLevel level);
			//! 部分的に書き込み
			/*!
				現状ではMipmap有りでの書き込みには非対応
				\param[in] rect 書き込み先座標
				\param[in] elem 入力フォーマット(Type)
			*/
			void writeRect(AB_Byte buff, const lubee::RectI& rect, GLTypeFmt elem);
			void writeRect(AB_Byte buff, const lubee::RectI& rect, GLTypeFmt elem, MipLevel level);

			bool isCubemap() const override;
			DEF_DEBUGGUI_NAME
	};
	class TextureSrc_MemCube :
		public TextureSrc_Mem
	{
		private:
			struct Cache {
				GLInFmt			baseFormat;
				GLTypeFmt		elemType;
				ByteBuff		pixels[6];

				Cache(GLInFmt baseFormat, GLTypeFmt elemType, lubee::SizeI size);
			};
			using Cache_Op = spi::Optional<Cache>;
			mutable Cache_Op	_cache;
			Cache& prepareCache(GLTypeFmt elem) const;

			void _backupBuffer() const override;
			void _restoreBuffer() override;

			template <class CB>
			void Iter(CB&& cb) const;
			void _writeData(AB_Byte buff, GLTypeFmt elem, CubeFace face, MipLevel level, bool mipAuto);
			void _writeRect(AB_Byte buff, const lubee::RectI& rect, GLTypeFmt elem, CubeFace face, MipLevel level, bool mipAuto);
		public:
			using TextureSrc_Mem::TextureSrc_Mem;
			//! テクスチャ全部書き換え = バッファも置き換え
			/*!
				\param[in] fmt テクスチャのフォーマット
				\param[in] elem 入力フォーマット(Type)
				\param[in] face Cubemapにおける面
			*/
			void writeData(AB_Byte buff, GLTypeFmt elem, CubeFace face);
			void writeData(AB_Byte buff, GLTypeFmt elem, CubeFace face, MipLevel level);
			//! 部分的に書き込み
			/*!
				現状ではMipmap有りでの書き込みには非対応
				\param[in] rect 書き込み先座標
				\param[in] elem 入力フォーマット(Type)
				\param[in] face Cubemapにおける面
			*/
			void writeRect(AB_Byte buff, const lubee::RectI& rect, GLTypeFmt elem, CubeFace face);
			void writeRect(AB_Byte buff, const lubee::RectI& rect, GLTypeFmt elem, CubeFace face, MipLevel level);

			bool isCubemap() const override;
			DEF_DEBUGGUI_NAME
	};
}
