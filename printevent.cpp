#include "sys_message.hpp"
#include "output.hpp"
#include <SDL_events.h>

namespace rev {
	const PrintEvent::TypeP PrintEvent::cs_type[NUM_EVENT] = {
		{[](const uint32_t id) { return id == SDL_WINDOWEVENT; }, Window},
		{[](const uint32_t id) { return id>=SDL_FINGERDOWN && id<= SDL_FINGERMOTION; }, Touch}
	};

	bool PrintEvent::Window(const SDL_Event& e) {
		switch(e.window.event) {
			case SDL_WINDOWEVENT_HIDDEN:
				LogR(Verbose, "Window %1% hidden", e.window.windowID); break;
			case SDL_WINDOWEVENT_EXPOSED:
				LogR(Verbose, "Window %1% exposed", e.window.windowID); break;
			case SDL_WINDOWEVENT_MOVED:
				LogR(Verbose, "Window %1% moved to %2%,%3%",
					e.window.windowID, e.window.data1,
					e.window.data2); break;
			case SDL_WINDOWEVENT_RESIZED:
				LogR(Verbose, "Window %1% resized to %2%x%3%",
                    e.window.windowID, e.window.data1,
                    e.window.data2); break;
			case SDL_WINDOWEVENT_MINIMIZED:
				LogR(Verbose, "Window %1% minimized", e.window.windowID); break;
			case SDL_WINDOWEVENT_MAXIMIZED:
				LogR(Verbose, "Window %1% maximized", e.window.windowID); break;
			case SDL_WINDOWEVENT_RESTORED:
				LogR(Verbose, "Window %1% restored", e.window.windowID); break;
			case SDL_WINDOWEVENT_ENTER:
				LogR(Verbose, "Mouse entered window %1%", e.window.windowID); break;
			case SDL_WINDOWEVENT_LEAVE:
				LogR(Verbose, "Mouse left window %1%", e.window.windowID); break;
			case SDL_WINDOWEVENT_FOCUS_GAINED:
				LogR(Verbose, "Window %1% gained keyboard focus",
                    e.window.windowID); break;
			case SDL_WINDOWEVENT_FOCUS_LOST:
				LogR(Verbose, "Window %1% lost keyboard focus",
                    e.window.windowID); break;
			case SDL_WINDOWEVENT_CLOSE:
				LogR(Verbose, "Window %1% shown", e.window.windowID); break;
			default:
				LogR(Verbose, "Window %1% got unknown event %2%",
                    e.window.windowID, e.window.event);
				return false;
		}
		return true;
	}
	bool PrintEvent::Touch(const SDL_Event& e) {
		const auto printCoord = [](const SDL_Event& e, const char* act) {
			auto& tf = e.tfinger;
			LogR(Verbose, "Touch %1%  finger %2% %3%", tf.touchId, act, tf.fingerId);
			LogR(Verbose, "x=%1% y=%2% dx=%3% dy=%4% pressure=%5%", tf.x, tf.y, tf.dx, tf.dy, tf.pressure);
		};
		switch(e.type) {
			case SDL_FINGERDOWN:
				printCoord(e, "down"); break;
			case SDL_FINGERUP:
				printCoord(e, "up"); break;
			case SDL_FINGERMOTION:
				printCoord(e, "move"); break;
			default:
				return false;
		}
		return true;
	}
	void PrintEvent::All(const SDL_Event& e, uint32_t filter) {
		for(auto& t : cs_type) {
			if(filter == 0)
				break;
			if(t.checker(e.type))
				t.proc(e);
			filter >>= 1;
		}
	}
}
