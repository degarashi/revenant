#include "lvalue.hpp"
#include "lubee/freelist.hpp"

namespace rev {
	// ------------------- LV_Global -------------------
	const std::string LV_Global::cs_entry("CS_ENTRY");
	lubee::Freelist<int> LV_Global::s_index(std::numeric_limits<int>::max(), 1);

	LV_Global::LV_Global(lua_State* ls) {
		_init(LuaState::GetMainLS_SP(ls));
	}
	LV_Global::LV_Global(const Lua_SP& sp, const LCValue& lcv) {
		lcv.push(sp->getLS());
		_init(sp);
	}
	LV_Global::LV_Global(const Lua_SP& sp) {
		_init(sp);
	}
	LV_Global::LV_Global(const LV_Global& lv) {
		lv._prepareValue(true);
		_init(lv._lua);
	}
	LV_Global::LV_Global(LV_Global&& lv):
		_lua(std::move(lv._lua)),
		_id(lv._id)
	{}
	LV_Global::~LV_Global() {
		if(_lua) {
			// エントリの削除
			_lua->getGlobal(cs_entry);
			_lua->setField(-1, _id, LuaNil());
			_lua->pop(1);
			s_index.put(_id);
		}
	}
	void LV_Global::_init(const Lua_SP& sp) {
		_lua = sp;
		_id = s_index.get();
		_setValue();
	}
	void LV_Global::_setValue() {
		// エントリの登録
		_lua->getGlobal(cs_entry);
		if(_lua->type(-1) != LuaType::Table) {
			_lua->pop(1);
			_lua->newTable();
			_lua->pushValue(-1);
			_lua->setGlobal(cs_entry);
		}
		_lua->push(_id);
		_lua->pushValue(-3);
		// [Value][Entry][id][Value]
		_lua->setTable(-3);
		_lua->pop(2);
	}

	int LV_Global::_prepareValue(bool /*bTop*/) const {
		_lua->getGlobal(cs_entry);
		_lua->getField(-1, _id);
		// [Entry][Value]
		_lua->remove(-2);
		return _lua->getTop();
	}
	void LV_Global::_prepareValue(lua_State* ls) const {
		VPop vp(*this, true);
		lua_State* mls = _lua->getLS();
		lua_xmove(mls, ls, 1);
	}
	void LV_Global::_cleanValue(const int pos) const {
		_lua->remove(pos);
	}
	lua_State* LV_Global::getLS() const {
		return _lua->getLS();
	}
	void LV_Global::swap(LV_Global& lv) noexcept {
		std::swap(_lua, lv._lua);
		std::swap(_id, lv._id);
	}

	// ------------------- LV_Stack -------------------
	LV_Stack::LV_Stack(lua_State* ls) {
		_init(ls);
	}
	LV_Stack::LV_Stack(lua_State* ls, const LCValue& lcv) {
		lcv.push(ls);
		_init(ls);
	}
	LV_Stack::~LV_Stack() {
		if(!std::uncaught_exception()) {
			D_Assert0(lua_gettop(_ls) >= _pos);
			#ifdef DEBUG
				D_Assert0(LuaState::SType(_ls, _pos) == _type);
			#endif
			lua_remove(_ls, _pos);
		}
	}
	void LV_Stack::_init(lua_State* ls) {
		_ls = ls;
		_pos = lua_gettop(ls);
		Assert0(_pos > 0);
		#ifdef DEBUG
			_type = LuaState::SType(ls, _pos);
		#endif
	}
	void LV_Stack::_setValue() {
		lua_replace(_ls, _pos);
		#ifdef DEBUG
			_type = LuaState::SType(_ls, _pos);
		#endif
	}
	LV_Stack& LV_Stack::operator = (const LCValue& lcv) {
		lcv.push(_ls);
		_setValue();
		return *this;
	}
	LV_Stack& LV_Stack::operator = (lua_State* ls) {
		if(ls != _ls) {
			lua_pushthread(ls);
			lua_xmove(ls, _ls, 1);
		} else
			lua_pushthread(_ls);
		_setValue();
		return *this;
	}

	int LV_Stack::_prepareValue(const bool bTop) const {
		if(bTop) {
			lua_pushvalue(_ls, _pos);
			return lua_gettop(_ls);
		}
		return _pos;
	}
	void LV_Stack::_prepareValue(lua_State* ls) const {
		_prepareValue(true);
		lua_xmove(_ls, ls, 1);
	}
	void LV_Stack::_cleanValue(const int pos) const {
		if(_pos < pos)
			lua_remove(_ls, pos);
	}
	lua_State* LV_Stack::getLS() const {
		return _ls;
	}

	std::ostream& operator << (std::ostream& os, const LV_Global& v) {
		typename LV_Global::VPop vp(v, true);
		return os << *v._lua;
	}
	std::ostream& operator << (std::ostream& os, const LV_Stack& v) {
		typename LV_Stack::VPop vp(v, true);
		return os << LCValue(v._ls);
	}
}
