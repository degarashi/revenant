#pragma once
#include "handle/input.hpp"
#include "input_constant.hpp"
#include <SDL_keyboard.h>

namespace rev {
	class SDLKeyboard {
		private:
			// 単純にSDLのフラグをコピー
			std::array<uint8_t, SDL_NUM_SCANCODES>	_state;
			static HInput s_hInput;
		protected:
			int dep_getButton(VKey::e num) const noexcept;
			bool dep_scan() noexcept;
			// キーボードは1つだけ想定
			template <class Maker>
			static HInput OpenKeyboard() {
				if(!s_hInput)
					s_hInput = Maker()();
				return s_hInput;
			}
			const std::string& name() const noexcept;
		public:
			static void Update() noexcept;
			static void Terminate() noexcept;
	};
	using KeyboardDep = SDLKeyboard;
}
