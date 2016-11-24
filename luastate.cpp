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
	LuaState::EType::EType(const char* typ0, const char* typ1):
		EBase("InvalidType", std::string("[") + typ0 + "] to [" + typ1 + "]")
	{}
	// ----------------- LuaState -----------------
	const std::string LuaState::cs_fromId("FromId"),
						LuaState::cs_fromThread("FromThread"),
						LuaState::cs_mainThread("MainThread");
	const int LuaState::ENT_ID = 1,
			  LuaState::ENT_THREAD = 2,
			  LuaState::ENT_NREF = 3,
			  LuaState::ENT_SPILUA = 4;
	lubee::Freelist<int> LuaState::s_index(std::numeric_limits<int>::max(), 1);
	void LuaState::Nothing(lua_State* /*ls*/) {}
	void LuaState::Delete(lua_State* ls) {
		// これが呼ばれる時は依存するスレッドなどが全て削除された時なので直接lua_closeを呼ぶ
		lua_close(ls);
	}
	LuaState::Deleter LuaState::_MakeDeleter(const int id) {
		return [id](lua_State* ls) {
			LuaState lsc(ls);
			_Decrement_ID(lsc, id);
		};
	}
	void LuaState::_PreapareThreadTable(LuaState& lsc) {
		// テーブルが存在しない場合は作成
		lsc.getGlobal(cs_fromId);
		lsc.getGlobal(cs_fromThread);
		if(!lsc.isTable(-1)) {
			lsc.pop(2);

			lsc.newTable();
			lsc.pushValue(-1);
			lsc.setGlobal(cs_fromId);

			lsc.newTable();
			lsc.pushValue(-1);
			lsc.setGlobal(cs_fromThread);
		}
	}
	ILua_SP LuaState::_Increment(LuaState& lsc, std::function<void (LuaState&)> cb) {
		const RewindTop rwt(lsc.getLS());

		_PreapareThreadTable(lsc);
		cb(lsc);
		// [fromId][fromThread][123]
		lsc.getField(-1, ENT_NREF);
		int nRef = lsc.toInteger(-1);
		lsc.setField(-2, ENT_NREF, ++nRef);
		lsc.getField(-2, ENT_SPILUA);
		ILua_SP ret = *reinterpret_cast<ILua_SP*>(lsc.toUserData(-1));

		return ret;
	}
	ILua_SP LuaState::_Increment_ID(LuaState& lsc, const int id) {
		return _Increment(lsc,
			[id](LuaState& lsc){
				// lsc[-2] = FromId
				// lsc[-1] = FromThread
				lsc.getField(-2, id);
			}
		);
	}
	ILua_SP LuaState::_Increment_Th(LuaState& lsc) {
		return _Increment(
			lsc,
			[pos=lsc.getTop()](LuaState& lsc){
				// lsc[-2] = FromId
				// lsc[-1] = FromThread
				lsc.pushValue(pos);
				lsc.getTable(-2);
			}
		);
	}
	void LuaState::_Decrement_ID(LuaState& lsc, const int id) {
		_Decrement(lsc, [id](LuaState& lsc){ lsc.getField(-2, id); });
	}
	void LuaState::_Decrement_Th(LuaState& lsc) {
		_Decrement(lsc, [pos=lsc.getTop()](LuaState& lsc){
			lsc.pushValue(pos);
			lsc.getTable(-2);
		});
	}
	void LuaState::_Decrement(LuaState& lsc, std::function<void (LuaState&)> cb) {
		const int top = lsc.getTop();

		_PreapareThreadTable(lsc);
		cb(lsc);
		// [fromId][fromThread][123]
		lsc.getField(-1, ENT_NREF);
		int nRef = lsc.toInteger(-1);
		if(--nRef == 0) {
			// エントリを消去
			lsc.getField(-2, ENT_ID);
			lsc.push(LuaNil());
			// [fromId][fromThread][123][nRef][ID][Nil]
			lsc.setTable(-6);
			// [fromId][fromThread][123][nRef]
			lsc.getField(-2, ENT_THREAD);
			lsc.push(LuaNil());
			// [fromId][fromThread][123][nRef][Thread][Nil]
			lsc.setTable(-5);
		} else {
			// デクリメントした値を代入
			lsc.setField(-2, ENT_NREF, nRef);
		}

		lsc.setTop(top);
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
	void LuaState::_registerNewThread(LuaState& lsc, const int id, const bool bBase) {
		const int top = lsc.getTop();

		_PreapareThreadTable(lsc);
		// G[id] = {1=Id, 2=Thread, 3=NRef, 4=ILua_SP-Ptr}
		lsc.newTable();
		lsc.setField(-1, ENT_ID, id);
		ILua_SP* ptr = &_lua;
		if(!bBase) {
			D_Assert0(!_lua);
			_lua = ILua_SP(lua_newthread(lsc.getLS()), _MakeDeleter(id));
		} else {
			D_Assert0(_lua);
			lsc.pushSelf();
		}
		lsc.push(ENT_THREAD);
		lsc.pushValue(-2);
		// [fromId][fromThread][123][Thread][ENT_THREAD][Thread]
		lsc.setTable(-4);
		// [fromId][fromThread][123][Thread]
		lsc.setField(-2, ENT_NREF, 1);

		lsc.push(ENT_SPILUA);
		lsc.push(ptr);
		// [fromId][fromThread][123][Thread][ENT_SPILUA][ptr]
		lsc.setTable(-4);

		// FromId
		lsc.push(id);
		lsc.pushValue(-3);
		// [fromId][fromThread][123][Thread][id][123]
		lsc.setTable(-6);

		// FromThread
		lsc.pushValue(-1);
		lsc.pushValue(-3);
		// [fromId][fromThread][123][Thread][Thread][123]
		lsc.setTable(-5);

		lsc.setTop(top);
	}

	LuaState::LuaState(const Lua_SP& spLua) {
		_id = s_index.get();

		_base = spLua->getMainLS_SP();
		_registerNewThread(*spLua, _id, false);
	}
	LuaState::LuaState(lua_State* ls, _TagThread) {
		LuaState lsc(ls);
		const int top = lsc.getTop();

		// 参照カウンタのインクリメント
		lsc.pushSelf();
		_lua = _Increment_Th(lsc);
		// メインスレッドのポインタを取得し、コピー
		lsc.getGlobal(cs_mainThread);
		void* pMain = lsc.toUserData(-1);
		_base = reinterpret_cast<LuaState*>(pMain)->shared_from_this();

		lsc.setTop(top);
	}

	LuaState::LuaState(const ILua_SP& ls):
		_lua(ls.get(), Nothing)  {}
	LuaState::LuaState(lua_State* ls):
		_lua(ls, Nothing) {}
	LuaState::LuaState(const lua_Alloc f, void* ud) {
		_lua = ILua_SP(f ? lua_newstate(f, ud) : luaL_newstate(), Delete);
		// メインスレッドの登録 -> global[cs_mainThread]
		push(static_cast<void*>(this));
		setGlobal(cs_mainThread);
		// スレッドリストにも加える
		_id = s_index.get();
		_registerNewThread(*this, _id, true);
	}
	LuaState::Reader::Reader(const HRW& hRW):
		ops(hRW),
		size(ops->size())
	{}
	Lua_SP LuaState::NewState(const lua_Alloc f, void* ud) {
		return Lua_SP(new LuaState(f, ud));
	}
	void LuaState::Reader::Read(lua_State* ls, const HRW& hRW, const char* chunkName, const char* mode) {
		Reader reader(hRW);
		int res = lua_load(ls, &Reader::Proc, &reader, (chunkName ? chunkName : ""), mode);
		LuaState::_CheckError(ls, res);
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
	LuaState::LuaState(LuaState&& ls): _base(std::move(ls._base)), _lua(std::move(ls._lua)) {}
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
	void LuaState::arith(OP op) {
		lua_arith(getLS(), static_cast<int>(op));
	}
	lua_CFunction LuaState::atPanic(const lua_CFunction panicf) {
		return lua_atpanic(getLS(), panicf);
	}
	int LuaState::call(const int nargs, const int nresults) {
		D_Scope(call)
			const int top = getTop() - 1;		// 1は関数の分
			const int err = lua_pcall(getLS(), nargs, nresults, 0);
			_checkError(err);
			return getTop() - top;
		D_ScopeEnd()
	}
	int LuaState::callk(const int nargs, const int nresults, lua_KContext ctx, lua_KFunction k) {
		D_Scope(callk)
			const int top = getTop() - 1;		// 1は関数の分
			const int err = lua_pcallk(getLS(), nargs, nresults, 0, ctx, k);
			_checkError(err);
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
	void LuaState::newTable(const int narr, const int nrec) {
		lua_createtable(getLS(), narr, nrec);
	}
	LuaState LuaState::newThread() {
		return LuaState(shared_from_this());
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
	void LuaState::rawGet(int idx, const LCValue& v) {
		if(idx < 0)
			idx = lua_absindex(getLS(), idx);
		D_Assert0(idx >= 0);
		push(v);
		rawGet(idx);
	}
	std::size_t LuaState::rawLen(const int idx) const {
		return lua_rawlen(getLS(), idx);
	}
	void LuaState::rawSet(const int idx) {
		lua_rawset(getLS(), idx);
	}
	void LuaState::rawSet(int idx, const LCValue& v) {
		if(idx < 0)
			idx = lua_absindex(getLS(), idx);
		D_Assert0(idx >= 0);
		push(v);
		rawSet(idx);
	}
	void LuaState::remove(const int idx) {
		lua_remove(getLS(), idx);
	}
	void LuaState::replace(const int idx) {
		lua_replace(getLS(), idx);
	}
	bool LuaState::resume(const Lua_SP& /*from*/, const int narg) {
		Lua_SP mls(getMainLS_SP());
		lua_State *ls0 = mls->getLS(),
					*ls1 = getLS();
		if(ls0 == ls1)
			return false;
		const int res = lua_resume(ls0, ls1, narg);
		_checkError(res);
		return res == LUA_YIELD;
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
		_checkError(res);
		return res != 0;
	}
	void LuaState::_checkType(const int idx, const LuaType typ) const {
		const LuaType t = type(idx);
		if(t != typ) {
			const std::string tmp0(typeName(t)),
							tmp1(typeName(typ));
			throw EType(tmp0.c_str(), tmp1.c_str());
		}
	}
	void LuaState::_CheckType(lua_State* ls, const int idx, const LuaType typ) {
		const LuaType t = SType(ls, idx);
		if(t != typ) {
			std::string tmp0(STypeName(ls, t)),
						tmp1(STypeName(ls, typ));
			throw EType(tmp0.c_str(), tmp1.c_str());
		}
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
	std::string LuaState::cnvString(const int idx) {
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
	void LuaState::prepareTable(const std::string& name) {
		getField(-1, name);
		if(type(-1) != LuaType::Table) {
			// テーブルを作成
			pop(1);
			newTable();
			push(name);
			pushValue(-2);
			// [Target][NewTable][name][NewTable]
			setTable(-4);
		}
	}
	void LuaState::prepareTableGlobal(const std::string& name) {
		pushGlobal();
		prepareTable(name);
		remove(-2);
	}
	lua_State* LuaState::getLS() const {
		return _lua.get();
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
		lua_getglobal(ls, cs_mainThread.c_str());
		void* ptr = lua_touserdata(ls, -1);
		Lua_SP sp = reinterpret_cast<LuaState*>(ptr)->shared_from_this();
		lua_pop(ls, 1);
		return sp;
	}
	void LuaState::_checkError(const int code) const {
		_CheckError(getLS(), code);
	}
	void LuaState::_CheckError(lua_State* ls, const int code) {
		if(code != LUA_OK) {
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
