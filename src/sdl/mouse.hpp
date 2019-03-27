#pragma once
#include "../input/sdl_const.hpp"
#include "../input/constant.hpp"
#include "../handle/input.hpp"
#include "lubee/src/error.hpp"
#include <SDL_mouse.h>

namespace rev {
	struct TPos2D;
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
}
