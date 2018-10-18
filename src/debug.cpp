#include "debug.hpp"
#include <boost/format.hpp>
#include <sstream>

namespace rev {
	std::string AddLineNumber(const std::string& src, const int numOffset, const int viewNum, const bool bPrevLR, const bool bPostLR) {
		std::string::size_type pos[2] = {};
		std::stringstream ss;
		if(bPrevLR)
			ss << std::endl;
		int lnum = numOffset;
		for(;;) {
			if(lnum >= viewNum)
				ss << (boost::format("%5d:\t") % lnum);
			else
				ss << "      \t";
			++lnum;
			pos[1] = src.find_first_of('\n', pos[0]);
			if(pos[1] == std::string::npos) {
				ss.write(&src[pos[0]], src.length()-pos[0]);
				break;
			}
			ss.write(&src[pos[0]], pos[1]-pos[0]);
			ss << std::endl;
			pos[0] = pos[1]+1;
		}
		if(bPostLR)
			ss << std::endl;
		return ss.str();
	}
}
