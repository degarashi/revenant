#include "input_dep_sdl.hpp"
#include <cstring>

namespace rev {
	HInput SDLKeyboard::s_hInput;
	int SDLKeyboard::dep_getButton(const int num) const noexcept {
		return (_state[num]) ? InputRange : 0;
	}
	bool SDLKeyboard::dep_scan() noexcept {
		std::memcpy(&_state[0], SDL_GetKeyboardState(nullptr), SDL_NUM_SCANCODES);
		return true;
	}
	void SDLKeyboard::Update() noexcept {}
	void SDLKeyboard::Terminate() noexcept {
		s_hInput.reset();
	}
	namespace {
		const std::string c_name("(default keyboard)");
	}
	const std::string& SDLKeyboard::name() const noexcept {
		return c_name;
	}
}
