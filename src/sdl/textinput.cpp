#include "textinput.hpp"
#include "../input/sdlvalue.hpp"

namespace rev {
	std::string SDLTextInput::s_text;
	void SDLTextInput::Update() {
		auto lc = g_sdlInputShared.lockC();
		s_text = lc->text;
	}
	const std::string& SDLTextInput::GetText() noexcept {
		return s_text;
	}
}
