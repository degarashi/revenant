#pragma once
#include <SDL_pixels.h>
#include <memory>
#include <initializer_list>
#include <cstdint>

namespace rev {
	struct RGB {
		union {
			uint8_t ar[3];
			struct { uint8_t r,g,b; };
		};
		RGB() = default;
		RGB(int r, int g, int b) noexcept;
	};
	struct RGBA {
		union {
			uint8_t ar[4];
			struct { uint8_t r,g,b,a; };
		};
		RGBA() = default;
		RGBA(RGB rgb, int a) noexcept;
		RGBA(const std::initializer_list<int>& il) noexcept;
	};

	using UPSDLFormat = std::unique_ptr<SDL_PixelFormat, decltype(&SDL_FreeFormat)>;
	UPSDLFormat MakeUPFormat(uint32_t fmt);
}
