#pragma once
#include "lubee/random.hpp"
#include "gameloopparam.hpp"
#include "sdl_mutex.hpp"

namespace rev {
	class Window;
	using Window_WP = std::weak_ptr<Window>;

	struct ShareData {
		lubee::RandomMT		mt;
		GameloopParam_UP	param;
		Window_WP			window;
		WFx					fx;

		ShareData();
	};
	template <class T>
	class SpinLock;
	extern SpinLock<ShareData> g_system_shared;
}
