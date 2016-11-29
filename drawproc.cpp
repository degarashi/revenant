#include "drawproc.hpp"
#include "sharedata.hpp"
#include "glx.hpp"
#include <cmath>

namespace rev {
	// ---------------- DrawProc ----------------
	bool DrawProc::runU(const uint64_t /*accum*/, const bool bSkip) {
		if(bSkip)
			return false;

		auto lk = g_system_shared.lock();
		if(auto fx = lk->fx.lock()) {
			static float a=0;
			GL.glClearColor(std::sin(a)/2+0.5f,std::sin(a*1.3)/2+0.5,0,0);
			a += 0.02f;
			GL.glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

			fx->execTask();
			return true;
		}
		return false;
	}
}
