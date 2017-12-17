#include "argchecker.hpp"
#include "gl_error.hpp"
#include "glx_parse.hpp"
#include "lubee/meta/countof.hpp"

namespace rev {
	// ----------------- ArgChecker -----------------
	ArgChecker::ArgChecker(std::ostream& ost, const std::string& shName, const std::vector<ArgItem>& args):_shName(shName), _ost(ost) {
		if(args.size() >= countof(_arg))
			throw GLE_InvalidArgument(_shName, "too many arguments");
		const int nA = args.size();
		for(int i=0 ; i<nA ; i++) {
			_target[i] = Detect(args[i].type);
			_arg[i] = &args[i];
		}
		for(int i=nA ; i<N_Target ; i++) {
			_target[i] = Target::None;
			_arg[i] = nullptr;
		}
	}
	ArgChecker::Target ArgChecker::Detect(const int type) {
		if(type <= GLType_::TYPE::boolT)
			return Target::Boolean;
		if(type <= GLType_::TYPE::floatT)
			return Target::Scalar;
		if(type <= GLType_::TYPE::ivec4T)
			return Target::Vector;
		return Target::None;
	}
	void ArgChecker::_checkAndSet(const Target tgt) {
		auto t = _target[_cursor];
		auto* arg = _arg[_cursor];
		if(t == Target::None)
			throw GLE_InvalidArgument(_shName, "(none)");
		if(t != tgt)
			throw GLE_InvalidArgument(_shName, arg->name);
		_ost << GLType_::cs_typeStr[arg->type] << ' ' << arg->name;
		++_cursor;
	}
	void ArgChecker::operator()(const std::vector<float>& v) {
		const int typ = _arg[_cursor]->type;
		_checkAndSet(Target::Vector);
		_ost << '=' << GLType_::cs_typeStr[typ] << '(';
		const int nV = v.size();
		for(int i=0 ; i<nV-1 ; i++)
			_ost << v[i] << ',';
		_ost << v.back() << ");" << std::endl;
	}
	void ArgChecker::operator()(const float v) {
		_checkAndSet(Target::Scalar);
		_ost << '=' << v << ';' << std::endl;
	}
	void ArgChecker::operator()(const bool b) {
		_checkAndSet(Target::Boolean);
		_ost << '=' << b << ';' << std::endl;
	}
	void ArgChecker::finalizeCheck() {
		if(_cursor >=static_cast<int>(countof(_target)))
			return;
		if(_target[_cursor] != Target::None)
			throw GLE_InvalidArgument(_shName, "(missing arguments)");
	}
}
