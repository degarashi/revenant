#include "../argchecker.hpp"
#include "../parse.hpp"
#include "lubee/src/meta/countof.hpp"

namespace rev {
	// ----------------- ArgChecker -----------------
	ArgChecker::ArgChecker(const std::string& shName, const ArgItemV& arg):
		_arg(arg),
		_shName(shName),
		_cursor(0)
	{}
	parse::Value::Type ArgChecker::_Detect(float) {
		return parse::Value::Type::Float;
	}
	parse::Value::Type ArgChecker::_Detect(int32_t) {
		return parse::Value::Type::Int;
	}
	parse::Value::Type ArgChecker::_Detect(bool) {
		return parse::Value::Type::Bool;
	}
	bool ArgChecker::_TypeCheck(const int from, const int to) {
		if(from == to)
			return true;
		if(from == parse::Value::Type::Int &&
			to == parse::Value::Type::Float)
			return true;
		return false;
	}
	void ArgChecker::finalizeCheck() {
		if(_cursor == int(_arg.size()))
			return;
		throw GLE_InvalidArgument(_shName, "(missing arguments)");
	}
}
