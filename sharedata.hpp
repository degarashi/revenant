#pragma once
#include "gameloopparam.hpp"
#include "spinlock.hpp"

namespace rev {
	class Window;
	using Window_WP = std::weak_ptr<Window>;

	struct ShareData {
		GameloopParam_UP	param;
		Window_WP			window;
		WFx					fx;

		ShareData();
	};
	extern SpinLock<ShareData> g_system_shared;
}
