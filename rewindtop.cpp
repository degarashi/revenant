#include "rewindtop.hpp"

namespace rev {
	RewindTop::RewindTop(lua_State* ls) noexcept:
		_ls(ls),
		_base(lua_gettop(ls)),
		_bReset(true)
	{}
	RewindTop::~RewindTop() {
		if(_bReset)
			lua_settop(_ls, _base);
	}
	void RewindTop::setReset(const bool r) noexcept {
		_bReset = r;
	}
	int RewindTop::getNStack() const noexcept {
		return lua_gettop(_ls) - _base;
	}
}
