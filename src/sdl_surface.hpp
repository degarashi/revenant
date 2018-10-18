#pragma once
#include "handle/sdl.hpp"
#include "sdl_mutex.hpp"
#include "abstbuffer.hpp"
#include "lubee/src/rect.hpp"
#include "handle/sdl.hpp"
#include <SDL_surface.h>
#include <memory>

namespace rev {
	struct RGB;
	struct RGBA;
	class Surface {
		private:
			SDL_Surface*	_sfc;
			mutable Mutex	_mutex;
			AB_Byte	_buff;
			class LockObj {
				const Surface& 	_sfc;
				void*		_bits;
				int			_pitch;

				public:
					LockObj(LockObj&& lk) noexcept;
					LockObj(const Surface& sfc, void* bits, int pitch) noexcept;
					~LockObj();
					void* getBits() noexcept;
					int getPitch() const noexcept;
					operator bool () const noexcept;
			};
			void _unlock() const noexcept;

			Surface(SDL_Surface* sfc) noexcept;
			Surface(SDL_Surface* sfc, ByteBuff&& buff) noexcept;
			ByteBuff _extractAsContinuous(uint32_t dstFmt=0) const;
		public:
			struct LoadFailed : std::runtime_error {
				using std::runtime_error::runtime_error;
			};
			static uint32_t Map(uint32_t format, RGB rgb) noexcept;
			//! RGBA値をSDLのピクセルフォーマット形式にパックする
			static uint32_t Map(uint32_t format, RGBA rgba);
			//! SDLのピクセルフォーマットからRGB値を取り出す
			static RGBA Get(uint32_t format, uint32_t pixel);
			//! SDLのピクセルフォーマット名を表す文字列を取得
			static const std::string& GetFormatString(uint32_t format);

			//! 任意のフォーマットの画像を読み込む
			static HSfc Load(HRW hRW);
			//! 空のサーフェス作成
			static HSfc Create(int w, int h, uint32_t format);
			//! ピクセルデータを元にサーフェス作成
			static HSfc Create(const ByteBuff& src, int pitch, int w, int h, uint32_t format);
			static HSfc Create(ByteBuff&& src, int pitch, int w, int h, uint32_t format);

			~Surface();
			void saveAsBMP(HRW hDst) const;
			void saveAsPNG(HRW hDst) const;
			LockObj lock() const;
			LockObj try_lock() const;
			const SDL_PixelFormat& getFormat() const noexcept;
			uint32_t getFormatEnum() const noexcept;
			lubee::SizeI getSize() const noexcept;
			int width() const noexcept;
			int height() const noexcept;
			//! 同サイズのサーフェスを作成
			HSfc makeBlank() const;
			HSfc duplicate() const;
			HSfc flipHorizontal() const;
			HSfc flipVertical() const;
			//! ピクセルフォーマット変換
			HSfc convert(uint32_t fmt) const;
			HSfc convert(const SDL_PixelFormat& fmt) const;
			//! ピクセルデータがデータ配列先頭から隙間なく詰められているか
			bool isContinuous() const noexcept;
			//! Continuousな状態でピクセルデータを抽出
			ByteBuff extractAsContinuous(uint32_t dstFmt=0) const;
			//! ビットブロック転送
			void blit(const HSfc& sfc, const lubee::RectI& srcRect, int dstX, int dstY) const;
			//! スケーリング有りのビットブロック転送
			void blitScaled(const HSfc& sfc, const lubee::RectI& srcRect, const lubee::RectI& dstRect) const;
			//! 単色での矩形塗りつぶし
			void fillRect(const lubee::RectI& rect, uint32_t color);
			SDL_Surface* getSurface() const noexcept;
			HSfc resize(const lubee::SizeI& s) const;
			void setEnableColorKey(uint32_t key);
			void setDisableColorKey();
			spi::Optional<uint32_t> getColorKey() const;
			void setBlendMode(SDL_BlendMode mode);
			SDL_BlendMode getBlendMode() const;
	};
}
