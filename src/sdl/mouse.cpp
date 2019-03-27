#include "input_sdlvalue.hpp"
#include "input_common.hpp"
#include "mouse.hpp"
#include <SDL_events.h>

namespace rev {
	HInput SDLMouse::s_hInput;
	SDL_Window* SDLMouse::s_window(nullptr);

	SDLMouse::SDLMouse() noexcept:
		_button{},
		_wheelDx(0),
		_wheelDy(0),
		_mode(MouseMode::Absolute)
	{}
	int SDLMouse::NumMouse() noexcept {
		return 1;
	}
	namespace {
		const std::string c_name("(default mouse)");
	}
	const std::string& SDLMouse::name() const noexcept {
		return c_name;
	}
	int SDLMouse::dep_numButtons() const noexcept {
		return N_SDLMouseButton;
	}
	int SDLMouse::dep_numAxes() const noexcept {
		return 4;
	}
	int SDLMouse::dep_getButton(const int num) const noexcept {
		return _button[num] ? InputRange : 0;
	}
	int SDLMouse::dep_getAxis(const int num) const noexcept {
		if(num < 2)
			return 0;
		return (num==2) ? _wheelDx : _wheelDy;
	}
	bool SDLMouse::dep_scan(TPos2D& t) NOEXCEPT_IF_RELEASE {
		int x, y;
		const auto state = D_SDLAssert(SDL_GetMouseState, &x, &y);
		{
			auto lc = g_sdlInputShared.lockC();
			for(int i=0 ; i<N_SDLMouseButton ; i++) {
				_button[i] = lc->button[i] | (state & SDL_BUTTON(SDLInputShared::c_buttonId[i]));
			}
			_wheelDx = lc->wheel_dx;
			_wheelDy = lc->wheel_dy;
		}
		// ウィンドウからカーソルが出ない様にする
		if(s_window) {
			if(_mode == MouseMode::Relative) {
				// カーソルを常にウィンドウ中央へセット
				int wx, wy;
				D_SDLAssert(SDL_GetWindowSize, s_window, &wx, &wy);
				wx >>= 1;
				wy >>= 1;
				D_SDLAssert(SDL_WarpMouseInWindow, s_window, wx, wy);
				t.setNewRel(frea::Vec2(x - wx, y - wy));
				return true;
			}
		}
		t.setNewAbs(frea::Vec2(x,y));
		// マウスは常に接続されている前提
		return true;
	}
	void SDLMouse::Terminate() noexcept {
		s_hInput.reset();
	}
	void SDLMouse::dep_setMode(MouseMode mode, TPos2D& t) NOEXCEPT_IF_RELEASE {
		_mode = mode;
		SDL_bool b = mode!=MouseMode::Absolute ? SDL_TRUE : SDL_FALSE;
		D_SDLAssert(SDL_SetWindowGrab, s_window, b);
		D_SDLAssert(SDL_ShowCursor, mode==MouseMode::Relative ? SDL_DISABLE : SDL_ENABLE);
		if(mode == MouseMode::Relative) {
			int wx, wy;
			D_SDLAssert(SDL_GetWindowSize, s_window, &wx, &wy);
			D_SDLAssert(SDL_WarpMouseInWindow, s_window, wx/2, wy/2);
			// カーソルをウィンドウ中央へセットした後に相対移動距離をリセット
			t.absPos = frea::Vec2(wx/2, wy/2);
			t.relPos = frea::Vec2(0);
		}
	}
	MouseMode SDLMouse::dep_getMode() const noexcept {
		return _mode;
	}
	void SDLMouse::SetWindow(SDL_Window* w) noexcept {
		s_window = w;
	}
}
