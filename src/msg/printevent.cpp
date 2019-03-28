#include "system.hpp"
#include "../output.hpp"
#include <SDL_events.h>

namespace rev {
	const PrintEvent::TypeP PrintEvent::cs_type[NUM_EVENT] = {
		{[](const uint32_t id) { return id == SDL_WINDOWEVENT; }, Window},
		{[](const uint32_t id) { return id>=SDL_FINGERDOWN && id<= SDL_FINGERMOTION; }, Touch}
	};

	namespace {
		template <class... Ts>
		bool PrintLog(const SDL_WindowEvent& w, const std::string& msg, Ts&&... ts) {
			std::string str("Window %1% ");
			str.append(msg);
			LogR(Verbose, str.c_str(), w.windowID, std::forward<Ts>(ts)...);
			return true;
		}
	}
	bool PrintEvent::Window(const SDL_Event& e) {
		const SDL_WindowEvent& w = e.window;
		switch(e.window.event) {
			case SDL_WINDOWEVENT_HIDDEN:
				return PrintLog(w, "hidden");
			case SDL_WINDOWEVENT_SHOWN:
				return PrintLog(w, "shown");
			case SDL_WINDOWEVENT_EXPOSED:
				return PrintLog(w, "exposed");
			case SDL_WINDOWEVENT_MOVED:
				return PrintLog(w, "moved to %2%,%3%", w.data1, w.data2);
			case SDL_WINDOWEVENT_RESIZED:
				return PrintLog(w, "resized to %2%,%3%", w.data1, w.data2);
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				return PrintLog(w, "size changed to %2%,%3%", w.data1, w.data2);
			case SDL_WINDOWEVENT_MINIMIZED:
				return PrintLog(w, "minimized");
			case SDL_WINDOWEVENT_MAXIMIZED:
				return PrintLog(w, "maximized");
			case SDL_WINDOWEVENT_RESTORED:
				return PrintLog(w, "restored");
			case SDL_WINDOWEVENT_ENTER:
				return PrintLog(w, "mouse entered");
			case SDL_WINDOWEVENT_LEAVE:
				return PrintLog(w, "mouse left");
			case SDL_WINDOWEVENT_FOCUS_GAINED:
				return PrintLog(w, "gained keyboard focus");
			case SDL_WINDOWEVENT_FOCUS_LOST:
				return PrintLog(w, "lost keyboard focus");
			case SDL_WINDOWEVENT_CLOSE:
				return PrintLog(w, "closed");
			case SDL_WINDOWEVENT_TAKE_FOCUS:
				return PrintLog(w, "takes focus");
			case SDL_WINDOWEVENT_HIT_TEST:
				return PrintLog(w, "hit tested (%2%, %3%)", w.data1, w.data2);
			default:
				PrintLog(w, "got unknown event data1=%2%, data2=%3%", w.data1, w.data2);
				return false;
		}
		return false;
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
