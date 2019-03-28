#pragma once
#include <string>
#include <iosfwd>

namespace rev {
	std::string AddLineNumber(const std::string& src, int numOffset, int viewNum, bool bPrevLR, bool bPostLR);
	void OutputCommentBlock(std::ostream& os, const std::string& msg);
}
