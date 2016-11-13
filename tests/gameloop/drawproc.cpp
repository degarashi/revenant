#include "main.hpp"
#include "../../gl_if.hpp"

namespace rev {
	namespace test {
		bool DrawProc::runU(const uint64_t /*accum*/, const bool /*bSkip*/) {
			GL.glClearColor(0.5f,0,0,0);
			GL.glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
			return true;
		}
	}
}
