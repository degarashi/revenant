#pragma once
#include "handle/input.hpp"
#include <SDL_joystick.h>

namespace rev {
	// SDLにてJoypadを列挙し、指定のパッドのインタフェースを生成
	class SDLJoypad {
		SDL_Joystick* 	_joypad;
		std::string		_name;
		uint32_t		_attachFlag;

		protected:
			int dep_numButtons() const noexcept;
			int dep_numAxes() const noexcept;
			int dep_numHats() const noexcept;
			int dep_getButton(int num) const noexcept;
			int dep_getAxis(int num) const noexcept;
			int dep_getHat(int num) const noexcept;
			bool dep_scan() noexcept;
			template <class Maker>
			static HInput OpenJoypad(const int num) {
				return Maker()(SDL_JoystickOpen(num));
			}
			const std::string& name() const noexcept;
		public:
			SDLJoypad(SDL_Joystick* jp) noexcept;
			~SDLJoypad();

			static void Initialize() noexcept;
			static void Update() noexcept;
			static void Terminate() noexcept;
			static int NumJoypad() noexcept;
	};
	using JoypadDep = SDLJoypad;
}
