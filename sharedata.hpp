#pragma once
#include "lubee/random.hpp"

namespace rev {
	class Window;
	using Window_WP = std::weak_ptr<Window>;
	struct GameloopParam;
	using GameloopParam_UP = std::unique_ptr<GameloopParam>;

	struct ShareData {
		lubee::RandomMT		mt;
		GameloopParam_UP	param;
		Window_WP			window;

		ShareData();
	};
	template <class T>
	class SpinLock;
	extern SpinLock<ShareData> g_system_shared;
}
