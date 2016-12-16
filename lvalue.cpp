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
	LV_Global::LV_Global(LV_Global&& lv) noexcept:
		_lua(std::move(lv._lua)),
		_id(lv._id)
	{}
	LV_Global::~LV_Global() {
		if(_lua) {
			const CheckTop ct(_lua->getLS());
			// エントリの削除
			_lua->pushValue(LUA_REGISTRYINDEX);
			_lua->getField(-1, cs_entry);
			_lua->setField(-1, _id, LuaNil());
			_lua->pop(2);
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
		_lua->prepareTableRegistry(cs_entry);
		_lua->push(_id);
		_lua->pushValue(-3);
		// [Value][Entry][id][Value]
		_lua->setTable(-3);
		_lua->pop(2);
	}

	int LV_Global::_prepareValue(bool /*bTop*/) const {
		_lua->pushValue(LUA_REGISTRYINDEX);
		_lua->getField(-1, cs_entry);
		_lua->getField(-1, _id);
		// [Registry][Entry][Value]
		_lua->remove(-2);
		_lua->remove(-2);
		return _lua->getTop();
	}
	lua_State* LV_Global::_prepareValue(lua_State* ls) const {
		_prepareValue(true);
		lua_State* mls = _lua->getLS();
		lua_xmove(mls, ls, 1);
		return ls;
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
	LV_Global& LV_Global::operator = (const LV_Global& lv) {
		lv._prepareValue(_lua->getLS());
		_setValue();
		return *this;
	}
	LV_Global& LV_Global::operator = (LV_Global&& lv) noexcept {
		this->~LV_Global();
		new(this) LV_Global(std::move(lv));
		return *this;
	}

	// ------------------- LV_Stack -------------------
	LV_Stack::LV_Stack(lua_State* ls) {
		_init(ls);
	}
	LV_Stack::LV_Stack(lua_State* ls, const LCValue& lcv) {
		lcv.push(ls);
		_init(ls);
	}
	LV_Stack::LV_Stack(LV_Stack&& lv) noexcept {
		_invalidate();
		swap(lv);
	}
	LV_Stack::~LV_Stack() {
		if(_ls && _pos>0 && !std::uncaught_exception()) {
			D_Assert0(lua_gettop(_ls) == _pos);
			#ifdef DEBUG
				D_Assert0(LuaState::SType(_ls, _pos) == _type);
			#endif
			lua_pop(_ls, 1);
		}
	}
	void LV_Stack::_invalidate() {
		_ls = nullptr;
		_pos = 0;
		#ifdef DEBUG
			_type = LuaType::Nil;
		#endif
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
		Assert0(_pos > 0);
		lua_replace(_ls, _pos);
		#ifdef DEBUG
			_type = LuaState::SType(_ls, _pos);
		#endif
	}
	int LV_Stack::_prepareValue(const bool bTop) const {
		if(bTop) {
			lua_pushvalue(_ls, _pos);
			return lua_gettop(_ls);
		}
		return _pos;
	}
	lua_State* LV_Stack::_prepareValue(lua_State* ls) const {
		_prepareValue(true);
		lua_xmove(_ls, ls, 1);
		return ls;
	}
	void LV_Stack::_cleanValue(const int pos) const {
		if(_pos < pos)
			lua_remove(_ls, pos);
	}
	lua_State* LV_Stack::getLS() const {
		return _ls;
	}
	void LV_Stack::swap(LV_Stack& lv) noexcept {
		std::swap(_ls, lv._ls);
		std::swap(_pos, lv._pos);
		#ifdef DEBUG
			std::swap(_type, lv._type);
		#endif
	}
	LV_Stack& LV_Stack::operator = (const LV_Stack& lv) {
		lv._prepareValue(_ls);
		_setValue();
		return *this;
	}
	LV_Stack& LV_Stack::operator = (LV_Stack&& lv) {
		this->~LV_Stack();
		new(this) LV_Stack(std::move(lv));
		return *this;
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
