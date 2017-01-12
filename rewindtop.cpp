#include "rewindtop.hpp"

namespace rev {
	// ------------- RewindTop -------------
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
	int RewindTop::getBase() const noexcept {
		return _base;
	}
	int RewindTop::getNStack() const noexcept {
		return lua_gettop(_ls) - _base;
	}

	// ------------- CheckTop -------------
	CheckTop::CheckTop(lua_State* ls) noexcept:
		_ls(ls),
		_base(lua_gettop(ls))
	{}
	CheckTop::~CheckTop() NOEXCEPT_IF_RELEASE {
		if(!std::uncaught_exception()) {
			D_Assert(lua_gettop(_ls) == _base,
					"stack top position differed: expect=%d, actual=%d", _base, lua_gettop(_ls));
		}
	}
	int CheckTop::getBase() const noexcept {
		return _base;
	}
}
