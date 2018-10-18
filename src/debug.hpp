#pragma once
#include <string>

namespace rev {
	std::string AddLineNumber(const std::string& src, int numOffset, int viewNum, bool bPrevLR, bool bPostLR);
}
