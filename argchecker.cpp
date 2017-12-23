#include "argchecker.hpp"
#include "glx_parse.hpp"
#include "lubee/meta/countof.hpp"

namespace rev {
	// ----------------- ArgChecker -----------------
	ArgChecker::ArgChecker(const std::string& shName, const ArgItemV& arg):
		_arg(arg),
		_shName(shName),
		_cursor(0)
	{}
	GLXValue::Type ArgChecker::_Detect(float) {
		return GLXValue::Type::Float;
	}
	GLXValue::Type ArgChecker::_Detect(int32_t) {
		return GLXValue::Type::Int;
	}
	GLXValue::Type ArgChecker::_Detect(bool) {
		return GLXValue::Type::Bool;
	}
	bool ArgChecker::_TypeCheck(const int from, const int to) {
		if(from == to)
			return true;
		if(from == GLXValue::Type::Int &&
			to == GLXValue::Type::Float)
			return true;
		return false;
	}
	void ArgChecker::finalizeCheck() {
		if(_cursor == int(_arg.size()))
			return;
		throw GLE_InvalidArgument(_shName, "(missing arguments)");
	}
}
