#include "sharedata.hpp"
#include "sdl/mutex.hpp"

namespace rev {
	// ---------------- ShareData ----------------
	// システムで使う共通変数
	ShareData::ShareData() {}
	SpinLock<ShareData>		g_system_shared;
}
