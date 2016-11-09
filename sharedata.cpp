#include "sharedata.hpp"
#include "sdl_mutex.hpp"

namespace rev {
	// ---------------- ShareData ----------------
	// システムで使う共通変数
	ShareData::ShareData():
		mt(lubee::RandomMT::Make<4>())
	{}
	SpinLock<ShareData>		g_system_shared;
}
