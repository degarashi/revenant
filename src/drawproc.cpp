#include "drawproc.hpp"
#include "sharedata.hpp"
#include "effect/effect.hpp"
#include <cmath>

namespace rev {
	// ---------------- DrawProc ----------------
	bool DrawProc::runU(const uint64_t /*accum*/) {
		HFx fx;
		{
			auto lk = g_system_shared.lock();
			fx = lk->fx.lock();
		}
		if(fx) {
			fx->execTask();
			return true;
		}
		return false;
	}
}
