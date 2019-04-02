#include "spec.hpp"
#include "../sdl/error.hpp"
#include <SDL_platform.h>
#include <SDL_cpuinfo.h>
#include <SDL_power.h>
#include <SDL_video.h>

namespace rev::info {
	namespace {
		using FeatFunc = SDL_bool (*)();
		const FeatFunc cs_ff[] = {
			SDL_Has3DNow,
			SDL_HasAltiVec,
			SDL_HasMMX,
			SDL_HasRDTSC,
			SDL_HasSSE,
			SDL_HasSSE2,
			SDL_HasSSE3,
			SDL_HasSSE41,
			SDL_HasSSE42,
			nullptr
		};
	}
	// ------------------ Spec::Display::Mode ------------------
	Spec::Display::Mode::Mode(const void* m) {
		const auto* mode = reinterpret_cast<const SDL_DisplayMode*>(m);
		format = mode->format;
		size = {
			mode->w,
			mode->h
		};
		rate = mode->refresh_rate;
	}
	bool Spec::Display::Mode::operator == (const Mode& m) const noexcept {
		return format == m.format &&
			size == m.size &&
			rate == m.rate;
	}
	// ------------------ Spec::Display::DPI ------------------
	Spec::Display::DPI::DPI(const int index) {
		D_SDLAssert(SDL_GetDisplayDPI, index, &diagonal, &horizontal, &vertical);
	}

	namespace {
		lubee::RectI ToRect(const SDL_Rect& r) {
			return {
				{r.x, r.y},
				{r.w, r.h}
			};
		}
	}
	// ------------------ Spec::Display ------------------
	int Spec::Display::NDisplay() noexcept {
		return D_SDLAssert(SDL_GetNumVideoDisplays);
	}
	Spec::Display Spec::Display::LoadInfo(const int index) {
		Display ret;
		ret.name = D_SDLAssert(SDL_GetDisplayName, index);

		SDL_DisplayMode mode;
		{
			const auto nMode = D_SDLAssert(SDL_GetNumDisplayModes, index);
			for(int i=0 ; i<nMode ; i++) {
				D_SDLAssert(SDL_GetDisplayMode, index, i, &mode);
				ret.mode.emplace_back(&mode);
			}
		}
		{
			D_SDLAssert(SDL_GetCurrentDisplayMode, index, &mode);
			const auto itr = std::find(ret.mode.begin(), ret.mode.end(), Mode(&mode));
			D_Assert0(itr != ret.mode.end());
			ret.currentModeIndex = static_cast<int>(ret.mode.end() - itr);
		}
		{
			D_SDLAssert(SDL_GetDesktopDisplayMode, index, &mode);
			const auto itr = std::find(ret.mode.begin(), ret.mode.end(), Mode(&mode));
			D_Assert0(itr != ret.mode.end());
			ret.desktopModeIndex = static_cast<int>(ret.mode.end() - itr);
		}
		{
			SDL_Rect rect;
			D_SDLAssert(SDL_GetDisplayBounds, index, &rect);
			ret.rect = ToRect(rect);
			D_SDLAssert(SDL_GetDisplayUsableBounds, index, &rect);
			ret.usableRect = ToRect(rect);
		}
		ret.dpi = DPI(index);
		return ret;
	}

	// ------------------ Spec ------------------
	Spec::Spec() noexcept:
		_platform(SDL_GetPlatform())
	{
		_nCacheLine = SDL_GetCPUCacheLineSize();
		_nCpu = SDL_GetCPUCount();
		_ramMB = SDL_GetSystemRAM();

		{
			Feature::value_t feat = 0;
			auto* f = cs_ff;
			Feature::value_t bit = 0x01;
			while(*f) {
				if((*f)())
					feat |= bit;
				bit <<= 1;
				++f;
			}
			_feature = feat;
		}
		{
			const auto nDisp = Display::NDisplay();
			_display.resize(nDisp);
			for(int i=0 ; i<nDisp ; i++)
				_display[i] = Display::LoadInfo(i);
		}
	}
	const Spec::Name& Spec::getPlatform() const noexcept {
		return _platform;
	}
	int Spec::cpuCacheLineSize() const noexcept {
		return _nCacheLine;
	}
	int Spec::cpuCount() const noexcept {
		return _nCpu;
	}
	int Spec::systemRAM() const noexcept {
		return _ramMB;
	}
	bool Spec::hasFeature(const PowerStatus::State flag) const noexcept {
		return _feature & flag;
	}
	Spec::PowerStatus Spec::powerStatus() const noexcept {
		PowerStatus ps;
		switch(SDL_GetPowerInfo(&ps.seconds, &ps.percentage)) {
			case SDL_POWERSTATE_ON_BATTERY:
				ps.state = PowerStatus::State::OnBattery; break;
			case SDL_POWERSTATE_NO_BATTERY:
				ps.state = PowerStatus::State::NoBattery; break;
			case SDL_POWERSTATE_CHARGING:
				ps.state = PowerStatus::State::Charging; break;
			case SDL_POWERSTATE_CHARGED:
				ps.state = PowerStatus::State::Charged; break;
			default:
				ps.state = PowerStatus::State::Unknown;
		}
		return ps;
	}

	void Spec::PowerStatus::output(std::ostream& os) const {
		if(state == PowerStatus::State::Unknown)
			os << "Unknown state";
		else if(state == PowerStatus::State::NoBattery)
			os << "No battery";
		else {
			os << "Battery state:" << std::endl;
			if(seconds == -1) os << "unknown time left";
			else os << seconds << " seconds left";
			os << std::endl;

			if(percentage == -1) os << "unknown percentage left";
			else os << percentage << " percent left";
		}
		os << std::endl;
	}
	const Spec::DisplayV& Spec::display() const noexcept {
		return _display;
	}
}
