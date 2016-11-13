#include "main.hpp"
#include "../../guithread.hpp"

namespace rev {
	namespace test {
		TEST_F(GameloopTest, General) {
			GUIThread loop(std::make_unique<Param>());
			loop.run();
		}
	}
}
