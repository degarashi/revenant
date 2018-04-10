#pragma once
#include "../../spinlock.hpp"

namespace rev::test {
	struct UserShare {};
	extern SpinLock<UserShare> g_shared;
}
