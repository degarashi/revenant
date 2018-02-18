#pragma once
#include "input_common.hpp"
#include "input_sdl_const.hpp"
#include <SDL_mouse.h>
#include <SDL_keyboard.h>
#include <SDL_joystick.h>
#include <SDL_events.h>
#include <string>
#include <bitset>
#include <vector>
#include <map>

namespace rev {
	class SDLKeyboard {
		// 単純にSDLのフラグをコピー
		std::array<uint8_t, SDL_NUM_SCANCODES>	_state;
		static HInput s_hInput;
		protected:
			int dep_getButton(int num) const noexcept;
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

	class SDLMouse {
		private:
			bool		_button[N_SDLMouseButton];
			int			_wheelDx,
						_wheelDy;
			MouseMode	_mode;
			// SDLのマウスは1つだけを想定
			// OpenMouseは共通のハンドルを返す
			static HInput		s_hInput;
			static SDL_Window*	s_window;
		protected:
			template <class Maker>
			static HInput OpenMouse(int /*num*/) {
				if(!s_hInput)
					s_hInput = Maker()();
				return s_hInput;
			}
			int dep_numButtons() const noexcept;
			int dep_numAxes() const noexcept;
			int dep_getButton(int num) const noexcept;
			int dep_getAxis(int num) const noexcept;
			bool dep_scan(TPos2D& t) NOEXCEPT_IF_RELEASE;
			void dep_setMode(MouseMode mode, TPos2D& t) NOEXCEPT_IF_RELEASE;
			MouseMode dep_getMode() const noexcept;
			const std::string& name() const noexcept;
			SDLMouse() noexcept;
		public:
			static void Terminate() noexcept;
			static int NumMouse() noexcept;
			static void SetWindow(SDL_Window* w) noexcept;
	};
	using MouseDep = SDLMouse;

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

	class SDLTouchpad {
		// TouchpadID -> Queue
		using EventQ = std::map<int, std::vector<SDL_Event>>;
		static EventQ	s_eventQ[2];
		static int		s_evSw;

		// FingerID -> PtrHandle
		using FMap = std::map<uint32_t, HPtr>;
		FMap	_fmap;
		int		_touchId;
		static HInput s_hInput;
		protected:
			template <class Maker>
			static HInput OpenTouchpad(const int num) {
				if(!s_hInput) {
					s_hInput = Maker()(num);
					_Initialize();
				}
				return s_hInput;
			}
			bool dep_scan(RecvPointer* r);
			static void _Initialize() noexcept;
			WPtr dep_getPointer() const;
			const std::string& name() const noexcept;
		public:
			SDLTouchpad(int touchId) noexcept;
			static void Update() noexcept;
			static void Terminate() noexcept;
			static int NumTouchpad() noexcept;
			static int ProcessEvent(void*, SDL_Event* e);
	};
	using TouchDep = SDLTouchpad;
}
