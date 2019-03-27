#include "keyboard.hpp"
#include "../input/sdlvalue.hpp"
#include <cstring>

namespace rev {
	namespace {
		// VKey to Scancode
		constexpr int Vkey2scancode[VKey::_Num] = {
			SDL_SCANCODE_A, SDL_SCANCODE_B, SDL_SCANCODE_C, SDL_SCANCODE_D, SDL_SCANCODE_E, SDL_SCANCODE_F, SDL_SCANCODE_G, SDL_SCANCODE_H, SDL_SCANCODE_I, SDL_SCANCODE_J, SDL_SCANCODE_K, SDL_SCANCODE_L, SDL_SCANCODE_M, SDL_SCANCODE_N, SDL_SCANCODE_O, SDL_SCANCODE_P, SDL_SCANCODE_Q, SDL_SCANCODE_R, SDL_SCANCODE_S, SDL_SCANCODE_T, SDL_SCANCODE_U, SDL_SCANCODE_V, SDL_SCANCODE_W, SDL_SCANCODE_X, SDL_SCANCODE_Y, SDL_SCANCODE_Z,
			SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3, SDL_SCANCODE_4, SDL_SCANCODE_5, SDL_SCANCODE_6, SDL_SCANCODE_7, SDL_SCANCODE_8, SDL_SCANCODE_9, SDL_SCANCODE_0,
			SDL_SCANCODE_RETURN, SDL_SCANCODE_ESCAPE, SDL_SCANCODE_BACKSPACE, SDL_SCANCODE_TAB, SDL_SCANCODE_SPACE,
			SDL_SCANCODE_MINUS, SDL_SCANCODE_EQUALS, SDL_SCANCODE_LEFTBRACKET, SDL_SCANCODE_RIGHTBRACKET, SDL_SCANCODE_BACKSLASH, SDL_SCANCODE_NONUSHASH, SDL_SCANCODE_SEMICOLON, SDL_SCANCODE_APOSTROPHE, SDL_SCANCODE_GRAVE, SDL_SCANCODE_COMMA, SDL_SCANCODE_PERIOD, SDL_SCANCODE_SLASH,
			SDL_SCANCODE_CAPSLOCK,
			SDL_SCANCODE_F1, SDL_SCANCODE_F2, SDL_SCANCODE_F3, SDL_SCANCODE_F4, SDL_SCANCODE_F5, SDL_SCANCODE_F6, SDL_SCANCODE_F7, SDL_SCANCODE_F8, SDL_SCANCODE_F9, SDL_SCANCODE_F10, SDL_SCANCODE_F11, SDL_SCANCODE_F12,
			SDL_SCANCODE_PRINTSCREEN, SDL_SCANCODE_SCROLLLOCK, SDL_SCANCODE_PAUSE, SDL_SCANCODE_INSERT,
			SDL_SCANCODE_HOME, SDL_SCANCODE_PAGEUP, SDL_SCANCODE_DELETE, SDL_SCANCODE_END, SDL_SCANCODE_PAGEDOWN, SDL_SCANCODE_RIGHT, SDL_SCANCODE_LEFT, SDL_SCANCODE_DOWN, SDL_SCANCODE_UP,
			SDL_SCANCODE_NUMLOCKCLEAR,
			SDL_SCANCODE_KP_DIVIDE, SDL_SCANCODE_KP_MULTIPLY, SDL_SCANCODE_KP_MINUS, SDL_SCANCODE_KP_PLUS, SDL_SCANCODE_KP_ENTER, SDL_SCANCODE_KP_1, SDL_SCANCODE_KP_2, SDL_SCANCODE_KP_3, SDL_SCANCODE_KP_4, SDL_SCANCODE_KP_5, SDL_SCANCODE_KP_6, SDL_SCANCODE_KP_7, SDL_SCANCODE_KP_8, SDL_SCANCODE_KP_9, SDL_SCANCODE_KP_0, SDL_SCANCODE_KP_PERIOD,
			SDL_SCANCODE_NONUSBACKSLASH, SDL_SCANCODE_APPLICATION, SDL_SCANCODE_POWER,
			SDL_SCANCODE_KP_EQUALS, SDL_SCANCODE_F13, SDL_SCANCODE_F14, SDL_SCANCODE_F15, SDL_SCANCODE_F16, SDL_SCANCODE_F17, SDL_SCANCODE_F18, SDL_SCANCODE_F19, SDL_SCANCODE_F20, SDL_SCANCODE_F21, SDL_SCANCODE_F22, SDL_SCANCODE_F23, SDL_SCANCODE_F24,
			SDL_SCANCODE_EXECUTE, SDL_SCANCODE_HELP, SDL_SCANCODE_MENU, SDL_SCANCODE_SELECT, SDL_SCANCODE_STOP, SDL_SCANCODE_AGAIN, SDL_SCANCODE_UNDO, SDL_SCANCODE_CUT, SDL_SCANCODE_COPY, SDL_SCANCODE_PASTE, SDL_SCANCODE_FIND, SDL_SCANCODE_MUTE, SDL_SCANCODE_VOLUMEUP, SDL_SCANCODE_VOLUMEDOWN,
			SDL_SCANCODE_KP_COMMA, SDL_SCANCODE_KP_EQUALSAS400,
			SDL_SCANCODE_INTERNATIONAL1, SDL_SCANCODE_INTERNATIONAL2, SDL_SCANCODE_INTERNATIONAL3, SDL_SCANCODE_INTERNATIONAL4, SDL_SCANCODE_INTERNATIONAL5, SDL_SCANCODE_INTERNATIONAL6, SDL_SCANCODE_INTERNATIONAL7, SDL_SCANCODE_INTERNATIONAL8, SDL_SCANCODE_INTERNATIONAL9,
			SDL_SCANCODE_LANG1, SDL_SCANCODE_LANG2, SDL_SCANCODE_LANG3, SDL_SCANCODE_LANG4, SDL_SCANCODE_LANG5, SDL_SCANCODE_LANG6, SDL_SCANCODE_LANG7, SDL_SCANCODE_LANG8, SDL_SCANCODE_LANG9,
			SDL_SCANCODE_ALTERASE, SDL_SCANCODE_SYSREQ, SDL_SCANCODE_CANCEL, SDL_SCANCODE_CLEAR, SDL_SCANCODE_PRIOR, SDL_SCANCODE_RETURN2, SDL_SCANCODE_SEPARATOR, SDL_SCANCODE_OUT, SDL_SCANCODE_OPER, SDL_SCANCODE_CLEARAGAIN, SDL_SCANCODE_CRSEL, SDL_SCANCODE_EXSEL,
			SDL_SCANCODE_KP_00, SDL_SCANCODE_KP_000, SDL_SCANCODE_THOUSANDSSEPARATOR, SDL_SCANCODE_DECIMALSEPARATOR, SDL_SCANCODE_CURRENCYUNIT, SDL_SCANCODE_CURRENCYSUBUNIT, SDL_SCANCODE_KP_LEFTPAREN, SDL_SCANCODE_KP_RIGHTPAREN, SDL_SCANCODE_KP_LEFTBRACE, SDL_SCANCODE_KP_RIGHTBRACE, SDL_SCANCODE_KP_TAB, SDL_SCANCODE_KP_BACKSPACE, SDL_SCANCODE_KP_A, SDL_SCANCODE_KP_B, SDL_SCANCODE_KP_C, SDL_SCANCODE_KP_D, SDL_SCANCODE_KP_E, SDL_SCANCODE_KP_F, SDL_SCANCODE_KP_XOR, SDL_SCANCODE_KP_POWER, SDL_SCANCODE_KP_PERCENT, SDL_SCANCODE_KP_LESS, SDL_SCANCODE_KP_GREATER, SDL_SCANCODE_KP_AMPERSAND, SDL_SCANCODE_KP_DBLAMPERSAND, SDL_SCANCODE_KP_VERTICALBAR, SDL_SCANCODE_KP_DBLVERTICALBAR, SDL_SCANCODE_KP_COLON, SDL_SCANCODE_KP_HASH, SDL_SCANCODE_KP_SPACE, SDL_SCANCODE_KP_AT, SDL_SCANCODE_KP_EXCLAM, SDL_SCANCODE_KP_MEMSTORE, SDL_SCANCODE_KP_MEMRECALL, SDL_SCANCODE_KP_MEMCLEAR, SDL_SCANCODE_KP_MEMADD, SDL_SCANCODE_KP_MEMSUBTRACT, SDL_SCANCODE_KP_MEMMULTIPLY, SDL_SCANCODE_KP_MEMDIVIDE, SDL_SCANCODE_KP_PLUSMINUS, SDL_SCANCODE_KP_CLEAR, SDL_SCANCODE_KP_CLEARENTRY, SDL_SCANCODE_KP_BINARY, SDL_SCANCODE_KP_OCTAL, SDL_SCANCODE_KP_DECIMAL, SDL_SCANCODE_KP_HEXADECIMAL,
			SDL_SCANCODE_LCTRL, SDL_SCANCODE_LSHIFT, SDL_SCANCODE_LALT, SDL_SCANCODE_LGUI, SDL_SCANCODE_RCTRL, SDL_SCANCODE_RSHIFT, SDL_SCANCODE_RALT, SDL_SCANCODE_RGUI,
			SDL_SCANCODE_MODE,
			SDL_SCANCODE_AUDIONEXT, SDL_SCANCODE_AUDIOPREV, SDL_SCANCODE_AUDIOSTOP, SDL_SCANCODE_AUDIOPLAY, SDL_SCANCODE_AUDIOMUTE, SDL_SCANCODE_MEDIASELECT, SDL_SCANCODE_WWW, SDL_SCANCODE_MAIL, SDL_SCANCODE_CALCULATOR, SDL_SCANCODE_COMPUTER, SDL_SCANCODE_AC_SEARCH, SDL_SCANCODE_AC_HOME, SDL_SCANCODE_AC_BACK, SDL_SCANCODE_AC_FORWARD, SDL_SCANCODE_AC_STOP, SDL_SCANCODE_AC_REFRESH, SDL_SCANCODE_AC_BOOKMARKS,
			SDL_SCANCODE_BRIGHTNESSDOWN, SDL_SCANCODE_BRIGHTNESSUP, SDL_SCANCODE_DISPLAYSWITCH,
			SDL_SCANCODE_KBDILLUMTOGGLE, SDL_SCANCODE_KBDILLUMDOWN, SDL_SCANCODE_KBDILLUMUP, SDL_SCANCODE_EJECT, SDL_SCANCODE_SLEEP,
			SDL_SCANCODE_APP1, SDL_SCANCODE_APP2,
		};
	}
	HInput SDLKeyboard::s_hInput;
	int SDLKeyboard::dep_getButton(const VKey::e num) const noexcept {
		return (_state[Vkey2scancode[num]]) ? InputRange : 0;
	}
	bool SDLKeyboard::dep_scan() noexcept {
		std::memcpy(&_state[0], SDL_GetKeyboardState(nullptr), SDL_NUM_SCANCODES);
		{
			auto lc = g_sdlInputShared.lock();
			lc->procKey([this](const int len, const KeyLog& k){
				if(len > 1 || k.down)
					_state[k.scancode] = 1;
			});
		}
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
