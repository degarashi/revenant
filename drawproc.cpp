#include "drawproc.hpp"
#include "sharedata.hpp"
#include "glx.hpp"
#include <cmath>

namespace rev {
	// ---------------- DrawProc ----------------
	bool DrawProc::runU(const uint64_t /*accum*/) {
		auto lk = g_system_shared.lock();
		if(auto fx = lk->fx.lock()) {
			fx->execTask();
			return true;
		}
		return false;
	}
}
