#include "lcv.hpp"
#include "lvalue.hpp"
#include "sdl_rw.hpp"
#include "apppath.hpp"
#include "lubee/freelist.hpp"
#include "rewindtop.hpp"
extern "C" {
	#include <lauxlib.h>
	#include <lualib.h>
}

namespace rev {
	// ----------------- LuaState::Exceptions -----------------
	LuaState::EBase::EBase(const std::string& typ_msg, const std::string& msg):
		std::runtime_error("")
	{
		std::stringstream ss;
		ss << "Error in LuaState:\nCause: " << typ_msg << std::endl << msg;
		reinterpret_cast<std::runtime_error&>(*this) = std::runtime_error(ss.str());
	}
	LuaState::ERun::ERun(const std::string& s):
		EBase("Runtime", s)
	{}
	LuaState::ESyntax::ESyntax(const std::string& s):
		EBase("Syntax", s)
	{}
	LuaState::EMem::EMem(const std::string& s):
		EBase("Memory", s)
	{}
	LuaState::EError::EError(const std::string& s):
		EBase("ErrorHandler", s)
	{}
	LuaState::EGC::EGC(const std::string& s):
		EBase("GC", s)
	{}
	LuaState::EType::EType(lua_State* ls, const LuaType expect, const LuaType actual):
		EBase(
			"InvalidType",
			std::string("[") + STypeName(ls, actual) +
			"] to [" + STypeName(ls, expect) + "]"
		),
		expect(expect),
		actual(actual)
	{}
	// ----------------- LuaState -----------------
	const std::string LuaState::cs_fromCpp("FromCpp"),
						LuaState::cs_fromLua("FromLua"),
						LuaState::cs_mainThreadPtr("MainThreadPtr"),
						LuaState::cs_mainThread("MainThread");

	void LuaState::Nothing(lua_State* /*ls*/) {}
	const LuaState::Deleter LuaState::s_deleter =
		[](lua_State* ls){
			// これが呼ばれる時はメインスレッドなどが全て削除された時なのでlua_closeを呼ぶ
			lua_close(ls);
		};
	LuaState::Deleter LuaState::_MakeCoDeleter(LuaState* lsp) {
		return [lsp](lua_State* ls) {
			D_Assert0(lsp->getLS() == ls);
			lsp->_unregisterCpp();
		};
	}
	void LuaState::_initMainThread() {
		const CheckTop ct(getLS());
		// メインスレッドの登録 -> global[cs_mainThreadPtr]
		push(static_cast<void*>(this));
		setGlobal(cs_mainThreadPtr);
		pushSelf();
		setGlobal(cs_mainThread);
		// fromCpp
		newTable();
		setGlobal(cs_fromCpp);
		// fromLua
		newTable();
		// FromLuaの方は弱参照テーブルにする
		newTable();
		setField(-1, "__mode", "k");
		// [fromLua][mode]
		setMetatable(-2);
		setGlobal(cs_fromLua);
	}
	void LuaState::_getThreadTable() {
		getGlobal(cs_fromCpp);
		getGlobal(cs_fromLua);
		D_Assert0(type(-2)==LuaType::Table &&
				type(-1)==LuaType::Table);
	}
	void LuaState::_registerLua() {
		D_Assert0(!_bWrap);
		const CheckTop ct(getLS());
		getGlobal(cs_fromLua);
		pushSelf();
		getTable(-2);
		if(type(-1) == LuaType::Nil) {
			pop(1);
			MakeUserdataWithDtor(*this, shared_from_this());
			// [fromLua][Lua_SP]
			pushSelf();
			pushValue(-2);
			// [fromLua][Lua_SP][Thread][Lua_SP]
			setTable(-4);
			pop(2);
		} else {
			// [fromLua][Lua_SP]
			D_Assert0(type(-1) == LuaType::Userdata);
			pop(2);
		}
	}
	void LuaState::_registerCpp() {
		D_Assert0(!_bWrap);
		const RewindTop rt(getLS());
		pushSelf();
		getGlobal(cs_fromCpp);
		// [Thread][fromCpp]
		push(reinterpret_cast<void*>(this));
		pushValue(rt.getBase()+1);
		// [Thread][fromCpp][LuaState*][Thread]
		D_Assert0(type(-1) == LuaType::Thread);
		setTable(-3);
	}
	void LuaState::_unregisterCpp() {
		D_Assert0(!_bWrap);
		const RewindTop rt(getLS());
		getGlobal(cs_fromCpp);
		// [fromCpp]
		push(reinterpret_cast<void*>(this));
		push(LuaNil{});
		// [fromCpp][LuaState*][Nil]
		setTable(-3);
	}
	Lua_SP LuaState::FromResource(const std::string& name) {
		Lua_SP sp;
		if(auto hlRW = mgr_path.getRW(luaNS::ScriptResourceEntry, PathBlock(name+"."+luaNS::ScriptExtension), Access::Read, nullptr)) {
			static spi::Optional<std::string> addpath;
			if(!addpath) {
				std::string path;
				const auto fnCollectPath = [&path](const std::string& ent){
					mgr_path.enumPath(ent, PathBlock(), [&path](const Dir& d){
						auto d2 = d;
						d2 <<= luaNS::system::PathReplaceMark;
						d2.setExtension(luaNS::ScriptExtension);
						path += luaNS::system::PathSeparation;
						path += d2.plain_utf8();
						return true;
					});
				};
				fnCollectPath(luaNS::ScriptResourceEntry);
				fnCollectPath(luaNS::SystemScriptResourceEntry);
				addpath = std::move(path);
			}
			sp = NewState();
			sp->loadLibraries();

			// パッケージのロードパスにアプリケーションリソースパスを追加
			sp->getGlobal(luaNS::system::Package);
			LValueS pkg(sp->getLS());
			pkg[luaNS::system::Path] = std::string(LValueS(pkg[luaNS::system::Path]).toString()) + *addpath;
			sp->load(hlRW);
		}
		return sp;
	}
	LuaState::LuaState(const Lua_SP& spLua) {
		_bWrap = false;
		lua_State* ls = spLua->getLS();
		const CheckTop ct(ls);
		_base = spLua->getMainLS_SP();
		_lua = ILua_SP(lua_newthread(ls), _MakeCoDeleter(this));
		_registerCpp();
		D_Assert0(getTop() == 0);
		spLua->pop(1);
	}
	LuaState::LuaState(lua_State* ls, const bool bCheckTop):
		_lua(ls, Nothing)
	{
		_bWrap = true;
		if(bCheckTop)
			_opCt = spi::construct(ls);
	}
	LuaState::LuaState(const lua_Alloc f, void* ud) {
		_bWrap = false;
		_lua = ILua_SP(f ? lua_newstate(f, ud) : luaL_newstate(), s_deleter);
		const CheckTop ct(_lua.get());
		_initMainThread();
	}
	LuaState::Reader::Reader(const HRW& hRW):
		ops(hRW),
		size(ops->size())
	{}
	Lua_SP LuaState::NewState(const lua_Alloc f, void* ud) {
		Lua_SP ret(new LuaState(f, ud));
		// スレッドリストにも加える
		ret->_registerCpp();
		ret->_registerLua();
		return ret;
	}
	// --------------- LuaState::Reader ---------------
	void LuaState::Reader::Read(lua_State* ls, const HRW& hRW, const char* chunkName, const char* mode) {
		Reader reader(hRW);
		const int res = lua_load(
			ls,
			&Reader::Proc,
			&reader,
			(chunkName ? chunkName : "(no name present)"),
			mode
		);
		LuaState::CheckError(ls, res);
	}
	const char* LuaState::Reader::Proc(lua_State* /*ls*/, void* data, std::size_t* size) {
		auto* self = reinterpret_cast<Reader*>(data);
		auto remain = self->size;
		if(remain > 0) {
			constexpr decltype(remain) BLOCKSIZE = 2048,
										MAX_BLOCK = 4;
			int nb, blocksize;
			if(remain <= BLOCKSIZE) {
				nb = 1;
				blocksize = *size = remain;
				self->size = 0;
			} else {
				nb = std::min(MAX_BLOCK, remain / BLOCKSIZE);
				blocksize = BLOCKSIZE;
				*size = BLOCKSIZE * nb;
				self->size -= *size;
			}
			self->buff.resize(*size);
			self->ops->read(self->buff.data(), blocksize, nb);
			return reinterpret_cast<const char*>(self->buff.data());
		}
		*size = 0;
		return nullptr;
	}
	const char* LuaState::cs_defaultmode = "bt";
	int LuaState::load(const HRW& hRW, const char* chunkName, const char* mode, const bool bExec) {
		Reader::Read(getLS(), hRW, chunkName, mode);
		if(bExec) {
			// Loadされたチャンクを実行
			return call(0, LUA_MULTRET);
		}
		return 0;
	}
	int LuaState::loadFromSource(const HRW& hRW, const char* chunkName, const bool bExec) {
		return load(hRW, chunkName, "t", bExec);
	}
	int LuaState::loadFromBinary(const HRW& hRW, const char* chunkName, const bool bExec) {
		return load(hRW, chunkName, "b", bExec);
	}
	int LuaState::loadModule(const std::string& name) {
		std::string s("require(\"");
		s.append(name);
		s.append("\")");
		HRW hRW = mgr_rw.fromConstTemporal(s.data(), s.length());
		return loadFromSource(hRW, name.c_str(), true);
	}
	void LuaState::pushSelf() {
		lua_pushthread(getLS());
	}
	void LuaState::loadLibraries() {
		luaL_openlibs(getLS());
	}

	void LuaState::push(const LCValue& v) {
		v.push(getLS());
	}
	void LuaState::pushCClosure(const lua_CFunction func, const int nvalue) {
		lua_pushcclosure(getLS(), func, nvalue);
	}
	void LuaState::pushValue(const int idx) {
		lua_pushvalue(getLS(), idx);
	}
	void LuaState::pop(const int n) {
		lua_pop(getLS(), n);
	}
	namespace {
		bool IsRegistryIndex(const int idx) noexcept {
			return idx==LUA_REGISTRYINDEX;
		}
	}
	int LuaState::absIndex(int idx) const {
		idx = lua_absindex(getLS(), idx);
		D_Assert0(IsRegistryIndex(idx) || idx>=0);
		return idx;
	}
	void LuaState::arith(const OP op) {
		lua_arith(getLS(), static_cast<int>(op));
	}
	lua_CFunction LuaState::atPanic(const lua_CFunction panicf) {
		return lua_atpanic(getLS(), panicf);
	}
	int LuaState::call(const int nargs, const int nresults) {
		D_Scope(call)
			const int top = getTop() - 1;		// 1は関数の分
			const int err = lua_pcall(getLS(), nargs, nresults, 0);
			checkError(err);
			return getTop() - top;
		D_ScopeEnd()
	}
	int LuaState::callk(const int nargs, const int nresults, lua_KContext ctx, lua_KFunction k) {
		D_Scope(callk)
			const int top = getTop() - 1;		// 1は関数の分
			const int err = lua_pcallk(getLS(), nargs, nresults, 0, ctx, k);
			checkError(err);
			return getTop() - top;
		D_ScopeEnd()
	}
	bool LuaState::checkStack(const int extra) {
		return lua_checkstack(getLS(), extra) != 0;
	}
	bool LuaState::compare(const int idx0, const int idx1, CMP cmp) const {
		return lua_compare(getLS(), idx0, idx1, static_cast<int>(cmp)) != 0;
	}
	void LuaState::concat(const int n) {
		lua_concat(getLS(), n);
	}
	void LuaState::copy(const int from, const int to) {
		lua_copy(getLS(), from, to);
	}
	void LuaState::dump(lua_Writer writer, void* data) {
		lua_dump(getLS(), writer, data, 0);
	}
	void LuaState::error() {
		lua_error(getLS());
	}
	int LuaState::gc(GC what, const int data) {
		return lua_gc(getLS(), static_cast<int>(what), data);
	}
	lua_Alloc LuaState::getAllocf(void** ud) const {
		return lua_getallocf(getLS(), ud);
	}
	void LuaState::getField(int idx, const LCValue& key) {
		idx = absIndex(idx);
		push(key);
		getTable(idx);
	}
	void LuaState::getGlobal(const LCValue& key) {
		pushGlobal();
		push(key);
		// [Global][key]
		getTable(-2);
		remove(-2);
	}
	const char* LuaState::getUpvalue(const int idx, const int n) {
		return lua_getupvalue(getLS(), idx, n);
	}
	void LuaState::getTable(const int idx) {
		lua_gettable(getLS(), idx);
	}
	int LuaState::getTop() const {
		return lua_gettop(getLS());
	}
	void LuaState::getUserValue(const int idx) {
		lua_getuservalue(getLS(), idx);
	}
	void LuaState::getMetatable(const int idx) {
		lua_getmetatable(getLS(), idx);
	}
	void LuaState::insert(const int idx) {
		lua_insert(getLS(), idx);
	}
	bool LuaState::isBoolean(const int idx) const {
		return lua_isboolean(getLS(), idx) != 0;
	}
	bool LuaState::isCFunction(const int idx) const {
		return lua_iscfunction(getLS(), idx) != 0;
	}
	bool LuaState::isLightUserdata(const int idx) const {
		return lua_islightuserdata(getLS(), idx) != 0;
	}
	bool LuaState::isNil(const int idx) const {
		return lua_isnil(getLS(), idx) != 0;
	}
	bool LuaState::isNone(const int idx) const {
		return lua_isnone(getLS(), idx) != 0;
	}
	bool LuaState::isNoneOrNil(const int idx) const {
		return lua_isnoneornil(getLS(), idx) != 0;
	}
	bool LuaState::isNumber(const int idx) const {
		return lua_isnumber(getLS(), idx) != 0;
	}
	bool LuaState::isString(const int idx) const {
		return lua_isstring(getLS(), idx) != 0;
	}
	bool LuaState::isTable(const int idx) const {
		return lua_istable(getLS(), idx) != 0;
	}
	bool LuaState::isThread(const int idx) const {
		return lua_isthread(getLS(), idx) != 0;
	}
	bool LuaState::isUserdata(const int idx) const {
		return lua_isuserdata(getLS(), idx) != 0;
	}
	void LuaState::length(const int idx) {
		lua_len(getLS(), idx);
	}
	int LuaState::getLength(const int idx) {
		length(idx);
		const int ret = toInteger(-1);
		pop(1);
		return ret;
	}
	void LuaState::newTable(const int narr, const int nrec) {
		lua_createtable(getLS(), narr, nrec);
	}
	Lua_SP LuaState::newThread() {
		return Lua_SP(new LuaState(shared_from_this()));
	}
	void* LuaState::newUserData(const std::size_t sz) {
		return lua_newuserdata(getLS(), sz);
	}
	int LuaState::next(const int idx) {
		return lua_next(getLS(), idx);
	}
	bool LuaState::rawEqual(const int idx0, const int idx1) {
		return lua_rawequal(getLS(), idx0, idx1) != 0;
	}
	void LuaState::rawGet(const int idx) {
		lua_rawget(getLS(), idx);
	}
	void LuaState::rawGetField(int idx, const LCValue& key) {
		if(idx < 0)
			idx = lua_absindex(getLS(), idx);
		D_Assert0(idx >= 0);
		push(key);
		rawGet(idx);
	}
	std::size_t LuaState::rawLen(const int idx) const {
		return lua_rawlen(getLS(), idx);
	}
	void LuaState::rawSet(const int idx) {
		lua_rawset(getLS(), idx);
	}
	void LuaState::rawSetField(int idx, const LCValue& key, const LCValue& val) {
		if(idx < 0)
			idx = lua_absindex(getLS(), idx);
		D_Assert0(idx >= 0);
		push(key);
		push(val);
		rawSet(idx);
	}
	void LuaState::remove(const int idx) {
		lua_remove(getLS(), idx);
	}
	void LuaState::replace(const int idx) {
		lua_replace(getLS(), idx);
	}
	std::pair<bool,int> LuaState::resume(const Lua_SP& from, const int narg) {
		lua_State *const ls = from ? from->getLS() : nullptr;
		const int res = lua_resume(getLS(), ls, narg);
		checkError(res);
		return std::make_pair(res==LUA_YIELD, getTop());
	}
	void LuaState::setAllocf(lua_Alloc f, void* ud) {
		lua_setallocf(getLS(), f, ud);
	}
	void LuaState::setField(int idx, const LCValue& key, const LCValue& val) {
		idx = absIndex(idx);
		push(key);
		push(val);
		setTable(idx);
	}
	void LuaState::setGlobal(const LCValue& key) {
		pushGlobal();
		push(key);
		pushValue(-3);
		// [value][Global][key][value]
		setTable(-3);
		pop(2);
	}
	void LuaState::setMetatable(const int idx) {
		lua_setmetatable(getLS(), idx);
	}
	void LuaState::pushGlobal() {
		pushValue(LUA_REGISTRYINDEX);
		getField(-1, LUA_RIDX_GLOBALS);
		// [Registry][Global]
		remove(-2);
	}
	void LuaState::setTable(const int idx) {
		lua_settable(getLS(), idx);
	}
	void LuaState::setTop(const int idx) {
		lua_settop(getLS(), idx);
	}
	void LuaState::setUservalue(const int idx) {
		lua_setuservalue(getLS(), idx);
	}
	const char* LuaState::setUpvalue(const int funcidx, const int n) {
		return lua_setupvalue(getLS(), funcidx, n);
	}
	void* LuaState::upvalueId(const int funcidx, const int n) {
		return lua_upvalueid(getLS(), funcidx, n);
	}
	void LuaState::upvalueJoin(const int funcidx0, const int n0, const int funcidx1, const int n1) {
		lua_upvaluejoin(getLS(), funcidx0, n0, funcidx1, n1);
	}
	bool LuaState::status() const {
		const int res = lua_status(getLS());
		checkError(res);
		return res != 0;
	}
	void LuaState::checkType(const int idx, const LuaType typ) const {
		const LuaType t = type(idx);
		if(t != typ)
			throw EType(getLS(), typ, t);
	}
	void LuaState::CheckType(lua_State* ls, const int idx, const LuaType typ) {
		LuaState lsc(ls, true);
		lsc.checkType(idx, typ);
	}
	bool LuaState::toBoolean(const int idx) const {
		return LCV<bool>()(idx, getLS());
	}
	lua_CFunction LuaState::toCFunction(const int idx) const {
		return LCV<lua_CFunction>()(idx, getLS());
	}
	lua_Integer LuaState::toInteger(const int idx) const {
		return LCV<lua_Integer>()(idx, getLS());
	}
	std::string LuaState::toString(const int idx) const {
		return LCV<std::string>()(idx, getLS());
	}
	std::string LuaState::cnvString(int idx) {
		idx = absIndex(idx);
		getGlobal(luaNS::ToString);
		pushValue(idx);
		call(1,1);
		std::string ret = toString(-1);
		pop(1);
		return ret;
	}
	lua_Number LuaState::toNumber(const int idx) const {
		return LCV<lua_Number>()(idx, getLS());
	}
	const void* LuaState::toPointer(const int idx) const {
		return lua_topointer(getLS(), idx);
	}
	Lua_SP LuaState::toThread(const int idx) const {
		return LCV<Lua_SP>()(idx, getLS());
	}
	void* LuaState::toUserData(const int idx) const {
		return LCV<void*>()(idx, getLS());
	}
	LCTable_SP LuaState::toTable(const int idx, LPointerSP* spm) const {
		return LCV<LCTable_SP>()(idx, getLS(), spm);
	}

	LCValue LuaState::toLCValue(const int idx, LPointerSP* spm) const {
		return LCV<LCValue>()(idx, getLS(), spm);
	}
	LuaType LuaState::type(const int idx) const {
		return SType(getLS(), idx);
	}
	LuaType LuaState::SType(lua_State* ls, const int idx) {
		const int typ = lua_type(ls, idx);
		return static_cast<LuaType::e>(typ);
	}

	const char* LuaState::typeName(const LuaType typ) const {
		return STypeName(getLS(), typ);
	}
	const char* LuaState::STypeName(lua_State* ls, const LuaType typ) {
		return lua_typename(ls,
				static_cast<int>(typ));
	}
	const lua_Number* LuaState::version() const {
		return lua_version(getLS());
	}
	void LuaState::xmove(const Lua_SP& to, const int n) {
		lua_xmove(getLS(), to->getLS(), n);
	}
	int LuaState::yield(const int nresults) {
		return lua_yield(getLS(), nresults);
	}
	int LuaState::yieldk(const int nresults, lua_KContext ctx, lua_KFunction k) {
		return lua_yieldk(getLS(), nresults, ctx, k);
	}
	bool LuaState::prepareTable(const int idx, const std::string& name) {
		getField(idx, name);
		if(type(-1) != LuaType::Table) {
			// テーブルを作成
			pop(1);
			newTable();
			push(name);
			pushValue(-2);
			// [Target][NewTable][name][NewTable]
			setTable(-4);
			return true;
		}
		return false;
	}
	bool LuaState::prepareTableGlobal(const std::string& name) {
		pushGlobal();
		const bool b = prepareTable(-1, name);
		remove(-2);
		return b;
	}
	lua_State* LuaState::getLS() const {
		return _lua.get();
	}
	Lua_SP LuaState::GetLS_SP(lua_State* ls) {
		Lua_SP ret;
		const RewindTop rt(ls);
		LuaState lsc(ls, false);
		lsc.getGlobal(cs_fromLua);
		lsc.pushSelf();
		lsc.getTable(-2);
		if(lsc.type(-1) == LuaType::Userdata) {
			// [fromLua][LuaState*]
			return (*reinterpret_cast<Lua_SP*>(lsc.toUserData(-1)))->shared_from_this();
		}
		lsc.pop(2);
		// Cppでのみ生きているスレッド
		lsc.getGlobal(cs_fromCpp);
		const int idx = lsc.getTop();
		// [fromCpp]
		lsc.push(LuaNil{});
		while(lsc.next(idx) != 0) {
			// key=-2 value=-1
			D_Assert0(lsc.type(-1) == LuaType::Thread);
			if(lua_tothread(ls, -1) == ls)
				return reinterpret_cast<LuaState*>(lsc.toUserData(-2))->shared_from_this();
			lsc.pop(1);
		}
		return Lua_SP();
	}
	Lua_SP LuaState::getLS_SP() {
		return shared_from_this();
	}
	Lua_SP LuaState::getMainLS_SP() {
		if(_base)
			return _base->getMainLS_SP();
		return shared_from_this();
	}
	Lua_SP LuaState::GetMainLS_SP(lua_State* ls) {
		const RewindTop rt(ls);
		lua_getglobal(ls, cs_mainThreadPtr.c_str());
		void* ptr = lua_touserdata(ls, -1);
		return reinterpret_cast<LuaState*>(ptr)->shared_from_this();
	}
	void LuaState::checkError(const int code) const {
		CheckError(getLS(), code);
	}
	void LuaState::CheckError(lua_State* ls, const int code) {
		const CheckTop ct(ls);
		if(code!=LUA_OK && code!=LUA_YIELD) {
			const char* msg = LCV<const char*>()(-1, ls);
			switch(code) {
				case LUA_ERRRUN:
					throw ERun(msg);
				case LUA_ERRMEM:
					throw EMem(msg);
				case LUA_ERRERR:
					throw EError(msg);
				case LUA_ERRSYNTAX:
					throw ESyntax(msg);
				case LUA_ERRGCMM:
					throw EGC(msg);
			}
			throw EBase("unknown error-code", msg);
		}
	}
	std::ostream& operator << (std::ostream& os, const LuaState& ls) {
		// スタックの値を表示する
		const int n = ls.getTop();
		for(int i=1 ; i<=n ; i++)
			os << "[" << i << "]: " << ls.toLCValue(i) << std::endl;
		return os;
	}
}
