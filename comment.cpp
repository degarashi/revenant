#include "comment.hpp"

namespace rev {
	void OutputCommentBlock(std::ostream& os, const std::string& msg) {
		const char* c_separate = "-------------";
		os << "// " << c_separate << ' ' << msg << ' ' << c_separate << std::endl;
	}
}
