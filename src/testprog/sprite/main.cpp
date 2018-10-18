#include "param.hpp"
#include "../test.hpp"
#include "../../guithread.hpp"

namespace rev {
	namespace test {
		struct GameloopTest : Random {};
		TEST_F(GameloopTest, General) {
			GUIThread loop(std::make_unique<Param>());
			loop.run();
		}
	}
}
