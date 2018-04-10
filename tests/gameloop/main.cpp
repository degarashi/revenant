#include "shared.hpp"
#include "param.hpp"
#include "../test.hpp"
#include "../../guithread.hpp"

namespace rev {
	namespace test {
		SpinLock<UserShare>	g_shared;
		struct GameloopTest : Random {};
		TEST_F(GameloopTest, General) {
			GUIThread loop(std::make_unique<Param>());
			loop.run();
		}
	}
}
