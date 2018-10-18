#include "sdl_color.hpp"
#include "lubee/src/error.hpp"

namespace rev {
	// ------------------------ RGB ------------------------
	RGB::RGB(const int r, const int g, const int b) noexcept:
		ar{uint8_t(r), uint8_t(g), uint8_t(b)}
	{}
	// ------------------------ RGBA ------------------------
	RGBA::RGBA(const RGB rgb, const int a) noexcept:
		ar{rgb.r, rgb.g, rgb.b, static_cast<uint8_t>(a)}
	{}
	RGBA::RGBA(const std::initializer_list<int>& il) noexcept {
		D_Assert0(il.size()==4);
		auto* pDst = ar;
		for(auto& v : il)
			*pDst++ = static_cast<uint8_t>(v);
	}

	UPSDLFormat MakeUPFormat(const uint32_t fmt) {
		return std::unique_ptr<SDL_PixelFormat, decltype(&SDL_FreeFormat)>(
				SDL_AllocFormat(fmt), SDL_FreeFormat);
	}
}
