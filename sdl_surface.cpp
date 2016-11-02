#include "sdl_surface.hpp"
#include "sdl_format.hpp"
#include "sdl_color.hpp"
#include "sdl_rw.hpp"
#include <SDL_image.h>

namespace rev {
	namespace {
		#define DEF_FSTR(name)	{name, #name},
		// フォーマット文字列
		const std::pair<uint32_t, std::string> c_formatName[] = {
			// 非デフォルト定義のフォーマット
			DEF_FSTR(SDL_PIXELFORMAT_R8)
			DEF_FSTR(SDL_PIXELFORMAT_RG88)
			DEF_FSTR(SDL_PIXELFORMAT_RGB101010)
			DEF_FSTR(SDL_PIXELFORMAT_BGR101010)
			DEF_FSTR(SDL_PIXELFORMAT_RGBA1010102)
			DEF_FSTR(SDL_PIXELFORMAT_ABGR2101010)
			DEF_FSTR(SDL_PIXELFORMAT_R16)
			DEF_FSTR(SDL_PIXELFORMAT_RG1616)
			DEF_FSTR(SDL_PIXELFORMAT_RGB161616)
			DEF_FSTR(SDL_PIXELFORMAT_BGR161616)
			DEF_FSTR(SDL_PIXELFORMAT_RGBA16161616)
			DEF_FSTR(SDL_PIXELFORMAT_ABGR16161616)
			DEF_FSTR(SDL_PIXELFORMAT_R32)
			DEF_FSTR(SDL_PIXELFORMAT_RG3232)
			DEF_FSTR(SDL_PIXELFORMAT_RGB323232)
			DEF_FSTR(SDL_PIXELFORMAT_BGR323232)
			DEF_FSTR(SDL_PIXELFORMAT_RGBA32323232)
			DEF_FSTR(SDL_PIXELFORMAT_ABGR32323232)
			DEF_FSTR(SDL_PIXELFORMAT_R16F)
			DEF_FSTR(SDL_PIXELFORMAT_RG1616F)
			DEF_FSTR(SDL_PIXELFORMAT_RGB161616F)
			DEF_FSTR(SDL_PIXELFORMAT_BGR161616F)
			DEF_FSTR(SDL_PIXELFORMAT_RGBA16161616F)
			DEF_FSTR(SDL_PIXELFORMAT_ABGR16161616F)
			DEF_FSTR(SDL_PIXELFORMAT_R32F)
			DEF_FSTR(SDL_PIXELFORMAT_RG3232F)
			DEF_FSTR(SDL_PIXELFORMAT_RGB323232F)
			DEF_FSTR(SDL_PIXELFORMAT_BGR323232F)
			DEF_FSTR(SDL_PIXELFORMAT_RGBA32323232F)
			DEF_FSTR(SDL_PIXELFORMAT_ABGR32323232F)
			// デフォルト定義のフォーマット
			DEF_FSTR(SDL_PIXELFORMAT_RGB332)
			DEF_FSTR(SDL_PIXELFORMAT_BGR555)
			DEF_FSTR(SDL_PIXELFORMAT_RGB555)
			DEF_FSTR(SDL_PIXELFORMAT_RGB444)
			DEF_FSTR(SDL_PIXELFORMAT_ABGR8888)
			DEF_FSTR(SDL_PIXELFORMAT_ARGB8888)
			DEF_FSTR(SDL_PIXELFORMAT_BGRA8888)
			DEF_FSTR(SDL_PIXELFORMAT_RGBA8888)
			DEF_FSTR(SDL_PIXELFORMAT_BGR888)
			DEF_FSTR(SDL_PIXELFORMAT_BGRX8888)
			DEF_FSTR(SDL_PIXELFORMAT_RGB888)
			DEF_FSTR(SDL_PIXELFORMAT_RGBX8888)
			DEF_FSTR(SDL_PIXELFORMAT_BGR24)
			DEF_FSTR(SDL_PIXELFORMAT_RGB24)
			DEF_FSTR(SDL_PIXELFORMAT_ARGB2101010)
			DEF_FSTR(SDL_PIXELFORMAT_BGR565)
			DEF_FSTR(SDL_PIXELFORMAT_RGB565)
			DEF_FSTR(SDL_PIXELFORMAT_ABGR1555)
			DEF_FSTR(SDL_PIXELFORMAT_ARGB1555)
			DEF_FSTR(SDL_PIXELFORMAT_BGRA5551)
			DEF_FSTR(SDL_PIXELFORMAT_RGBA5551)
			DEF_FSTR(SDL_PIXELFORMAT_ABGR4444)
			DEF_FSTR(SDL_PIXELFORMAT_ARGB4444)
			DEF_FSTR(SDL_PIXELFORMAT_BGRA4444)
			DEF_FSTR(SDL_PIXELFORMAT_RGBA4444)
		};
		#undef DEF_FSTR
	}
	uint32_t Surface::Map(const uint32_t format, const RGB rgb) noexcept {
		return Map(format, RGBA(rgb, 255));
	}
	uint32_t Surface::Map(const uint32_t format, const RGBA rgba) {
		const auto fmt = MakeUPFormat(format);
		return SDLAssert(SDL_MapRGBA, fmt.get(), rgba.r, rgba.g, rgba.b, rgba.a);
	}
	RGBA Surface::Get(const uint32_t format, const uint32_t pixel) {
		const auto fmt = MakeUPFormat(format);
		RGBA ret;
		SDLAssert(SDL_GetRGBA, pixel, fmt.get(), &ret.r, &ret.g, &ret.b, &ret.a);
		return ret;
	}
	const std::string& Surface::GetFormatString(const uint32_t format) {
		for(auto& p : c_formatName) {
			if(p.first == format)
				return p.second;
		}
		const static std::string c_unknown("SDL_PIXELFORMAT_UNKNOWN");
		return c_unknown;
	}

	// -------------------- Surface --------------------
	Surface::Surface(SDL_Surface* sfc) noexcept:
		_sfc(sfc),
		_buff(nullptr, 0)
	{}
	Surface::Surface(SDL_Surface* sfc, ByteBuff&& buff) noexcept:
		_sfc(sfc),
		_buff(std::move(buff))
	{}
	Surface::~Surface() {
		SDLWarn(SDL_FreeSurface, _sfc);
	}
	SPSurface Surface::Create(const int w, const int h, const uint32_t format) {
		auto fmt = MakeUPFormat(format);
		auto* sfc = SDLAssert(SDL_CreateRGBSurface, 0, w, h, fmt->BitsPerPixel, fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
		return SPSurface(new Surface(sfc));
	}
	SPSurface Surface::Create(const ByteBuff& src, const int pitch, const int w, const int h, const uint32_t format) {
		return Create(ByteBuff(src), pitch, w, h, format);
	}
	SPSurface Surface::Create(ByteBuff&& src, int pitch, const int w, const int h, const uint32_t format) {
		const auto fmt = MakeUPFormat(format);
		if(pitch==0)
			pitch = fmt->BytesPerPixel*w;
		auto* sfc = SDLAssert(SDL_CreateRGBSurfaceFrom, &src[0], w, h, fmt->BitsPerPixel, pitch, fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
		return SPSurface(new Surface(sfc, std::move(src)));
	}
	SPSurface Surface::Load(HRW hRW) {
		SDL_RWops* rw = hRW->getOps();
		IMGThrow(LoadFailed, IMG_Load_RW, rw, 0);
		IMGError err;
		err.reset();
		SDL_Surface* sfc = IMG_Load_RW(rw, 0);
		if(const char* e = err.errorDesc())
			throw LoadFailed(e);
		return SPSurface(new Surface(sfc));
	}
	void Surface::saveAsBMP(HRW hDst) const {
		SDLThrow(std::runtime_error, SDL_SaveBMP_RW, _sfc, hDst->getOps(), 0);
	}
	void Surface::saveAsPNG(HRW hDst) const {
		IMGThrow(std::runtime_error, IMG_SavePNG_RW, _sfc, hDst->getOps(), 0);
	}
	void Surface::fillRect(const lubee::RectI& rect, const uint32_t color) {
		SDL_Rect r;
		r.x = rect.x0;
		r.y = rect.y0;
		r.w = rect.width();
		r.h = rect.height();
		D_SDLAssert(SDL_FillRect, _sfc, &r, color);
	}
	Surface::LockObj Surface::lock() const {
		_mutex.lock();
		D_SDLAssert(SDL_LockSurface, _sfc);
		return LockObj(*this, _sfc->pixels, _sfc->pitch);
	}
	Surface::LockObj Surface::try_lock() const {
		if(_mutex.try_lock()) {
			D_SDLAssert(SDL_LockSurface, _sfc);
			return LockObj(*this, _sfc->pixels, _sfc->pitch);
		}
		return LockObj(*this, nullptr, 0);
	}
	void Surface::_unlock() const noexcept {
		D_SDLWarn(SDL_UnlockSurface, _sfc);
		_mutex.unlock();
	}
	lubee::SizeI Surface::getSize() const noexcept {
		return lubee::SizeI(_sfc->w, _sfc->h);
	}
	uint32_t Surface::getFormatEnum() const noexcept {
		return _sfc->format->format;
	}
	const SDL_PixelFormat& Surface::getFormat() const noexcept {
		return *_sfc->format;
	}
	int Surface::width() const noexcept {
		return _sfc->w;
	}
	int Surface::height() const noexcept {
		return _sfc->h;
	}
	SPSurface Surface::convert(const uint32_t fmt) const {
		SDL_Surface* nsfc = SDL_ConvertSurfaceFormat(_sfc, fmt, 0);
		Assert0(nsfc);
		return SPSurface(new Surface(nsfc));
	}
	SPSurface Surface::convert(const SDL_PixelFormat& fmt) const {
		SDL_Surface* nsfc = SDL_ConvertSurface(_sfc, const_cast<SDL_PixelFormat*>(&fmt), 0);
		Assert0(nsfc);
		return SPSurface(new Surface(nsfc));
	}
	bool Surface::isContinuous() const noexcept {
		return _sfc->pitch == _sfc->w * _sfc->format->BytesPerPixel;
	}
	ByteBuff Surface::extractAsContinuous(uint32_t dstFmt) const {
		auto& myformat = getFormat();
		if(dstFmt == 0)
			dstFmt = myformat.format;

		auto lk = lock();
		int w = width(),
			h = height();
		// ピクセルデータが隙間なく詰まっていて、なおかつフォーマットも同じならそのままメモリをコピー
		if(isContinuous() && dstFmt==myformat.format) {
			auto* src = reinterpret_cast<const uint8_t*>(lk.getBits());
			return ByteBuff(src, src + w*h*myformat.BytesPerPixel);
		}
		auto upFmt = MakeUPFormat(dstFmt);
		size_t dstSize = w * h * upFmt->BytesPerPixel;
		ByteBuff dst(dstSize);
		SDLAssert(SDL_ConvertPixels,
					w,h,
					myformat.format, lk.getBits(), lk.getPitch(),
					dstFmt, &dst[0], w*upFmt->BytesPerPixel);
		return dst;
	}
	SDL_Surface* Surface::getSurface() const noexcept {
		return _sfc;
	}
	namespace {
		SDL_Rect ToSDLRect(const lubee::RectI& r) noexcept {
			SDL_Rect ret;
			ret.x = r.x0;
			ret.y = r.y0;
			ret.w = r.width();
			ret.h = r.height();
			return ret;
		}
	}
	void Surface::blit(const SPSurface& sfc, const lubee::RectI& srcRect, const int dstX, const int dstY) const {
		SDL_Rect sr = ToSDLRect(srcRect),
				dr;
		dr.x = dstX;
		dr.y = dstY;
		dr.w = dr.w;
		dr.h = dr.h;
		SDLAssert(SDL_BlitSurface, _sfc, &sr, sfc->getSurface(), &dr);
	}
	void Surface::blitScaled(const SPSurface& sfc, const lubee::RectI& srcRect, const lubee::RectI& dstRect) const {
		SDL_Rect sr = ToSDLRect(srcRect),
				dr = ToSDLRect(dstRect);
		SDLAssert(SDL_BlitScaled, _sfc, &sr, sfc->getSurface(), &dr);
	}
	SPSurface Surface::resize(const lubee::SizeI& s) const {
		const auto bm = getBlendMode();
		auto* self = const_cast<Surface*>(this);
		self->setBlendMode(SDL_BLENDMODE_NONE);

		const auto sz = getSize();
		lubee::RectI srcRect(0,sz.width, 0,sz.height),
					dstRect(0,s.width, 0,s.height);
		SPSurface nsfc = Create(s.width, s.height, getFormat().format);
		blitScaled(nsfc, srcRect, dstRect);

		self->setBlendMode(bm);
		return nsfc;
	}
	SPSurface Surface::makeBlank() const {
		return Create(width(), height(), getFormatEnum());
	}
	SPSurface Surface::duplicate() const {
		auto sp = makeBlank();
		blit(sp, {0,width(),0,height()}, 0, 0);
		return sp;
	}
	SPSurface Surface::flipHorizontal() const {
		auto sp = makeBlank();
		const int w = width(),
					h = height();
		for(int i=0 ; i<w ; i++)
			blit(sp, {i,i+1,0,h}, w-i-1, 0);
		return sp;
	}
	SPSurface Surface::flipVertical() const {
		auto sp = makeBlank();
		const int w = width(),
					h = height();
		for(int i=0 ; i<h ; i++)
			blit(sp, {0,w,i,i+1}, 0, h-i-1);
		return sp;
	}
	void Surface::setEnableColorKey(const uint32_t key) {
		SDLAssert(SDL_SetColorKey, _sfc, SDL_TRUE, key);
	}
	void Surface::setDisableColorKey() {
		SDLAssert(SDL_SetColorKey, _sfc, SDL_FALSE, 0);
	}
	spi::Optional<uint32_t> Surface::getColorKey() const {
		uint32_t key;
		if(SDLAssert(SDL_GetColorKey, _sfc, &key) == -1)
			return spi::none;
		return key;
	}
	void Surface::setBlendMode(const SDL_BlendMode mode) {
		SDLAssert(SDL_SetSurfaceBlendMode, _sfc, mode);
	}
	SDL_BlendMode Surface::getBlendMode() const {
		SDL_BlendMode mode;
		const int ret = SDLAssert(SDL_GetSurfaceBlendMode, _sfc, &mode);
		Expect(ret >= 0, "unknown error");
		return mode;
	}

	// ----------------------------- Surface::LockObj -----------------------------
	Surface::LockObj::LockObj(const Surface& sfc, void* bits, const int pitch) noexcept:
		_sfc(sfc),
		_bits(bits),
		_pitch(pitch)
	{}
	Surface::LockObj::LockObj(LockObj&& lk) noexcept:
		_sfc(lk._sfc),
		_bits(lk._bits),
		_pitch(lk._pitch)
	{}
	Surface::LockObj::~LockObj() {
		if(_bits)
			_sfc._unlock();
	}
	Surface::LockObj::operator bool() const noexcept {
		return _bits != nullptr;
	}
	void* Surface::LockObj::getBits() noexcept {
		return _sfc._sfc->pixels;
	}
	int Surface::LockObj::getPitch() const noexcept {
		return _sfc._sfc->pitch;
	}
}
