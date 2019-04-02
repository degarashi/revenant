#include "spec.hpp"
#include <SDL_platform.h>
#include <SDL_cpuinfo.h>
#include <SDL_power.h>

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

	Spec::Spec() noexcept:
		_platform(SDL_GetPlatform())
	{
		_nCacheLine = SDL_GetCPUCacheLineSize();
		_nCpu = SDL_GetCPUCount();

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
	const std::string& Spec::getPlatform() const noexcept {
		return _platform;
	}
	int Spec::cpuCacheLineSize() const noexcept {
		return _nCacheLine;
	}
	int Spec::cpuCount() const noexcept {
		return _nCpu;
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
}
