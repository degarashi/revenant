#include "lvalue.hpp"
#include "../emplace.hpp"
#include "rewindtop.hpp"
#include "lubee/src/rect.hpp"

namespace rev {
	DEF_LCV_OSTREAM(void)
	DEF_LCV_OSTREAM(const std::string&)
	DEF_LCV_OSTREAM(lubee::SizeI)
	DEF_LCV_OSTREAM(lubee::RectI)
	DEF_LCV_OSTREAM(lua_OtherNumber)
	DEF_LCV_OSTREAM(lua_IntegerU)
	DEF_LCV_OSTREAM(lua_OtherInteger)
	DEF_LCV_OSTREAM(lua_OtherIntegerU)
	DEF_LCV_OSTREAM(long)
	DEF_LCV_OSTREAM(GLFormat)
	DEF_LCV_OSTREAM(GLDepthFmt)
	DEF_LCV_OSTREAM(GLStencilFmt)
	DEF_LCV_OSTREAM(GLDSFmt)
	DEF_LCV_OSTREAM(GLInFmt)
	DEF_LCV_OSTREAM(GLInSizedFmt)
	DEF_LCV_OSTREAM(GLInCompressedFmt)
	DEF_LCV_OSTREAM(GLInRenderFmt)
	DEF_LCV_OSTREAM(GLInReadFmt)
	DEF_LCV_OSTREAM(GLTypeFmt)

	// [LCV<LuaNil> = LUA_TNIL]
	void LCV<LuaNil>::operator()(lua_State* ls, LuaNil) const {
		lua_pushnil(ls);
	}
	LuaNil LCV<LuaNil>::operator()(const int idx, lua_State* ls, LPointerSP* /*spm*/) const {
		LuaState::CheckType(ls, idx, LuaType::Nil);
		return LuaNil();
	}
	LuaType LCV<LuaNil>::operator()(LuaNil) const {
		return LuaType::Nil;
	}
	DEF_LCV_OSTREAM(LuaNil)

	// [LCV<bool> = LUA_TBOOL]
	void LCV<bool>::operator()(lua_State* ls, const bool b) const {
		lua_pushboolean(ls, b);
	}
	bool LCV<bool>::operator()(const int idx, lua_State* ls, LPointerSP* /*spm*/) const {
		LuaState::CheckType(ls, idx, LuaType::Boolean);
		return lua_toboolean(ls, idx) != 0;
	}
	LuaType LCV<bool>::operator()(bool) const {
		return LuaType::Boolean;
	}
	DEF_LCV_OSTREAM(bool)

	// [LCV<const char*> = LUA_TSTRING]
	void LCV<const char*>::operator()(lua_State* ls, const char* c) const {
		if(c)
			lua_pushstring(ls, c);
		else
			lua_pushnil(ls);
	}
	const char* LCV<const char*>::operator()(const int idx, lua_State* ls, LPointerSP* /*spm*/) const {
		if(lua_type(ls, idx) == LUA_TNIL)
			return nullptr;
		LuaState::CheckType(ls, idx, LuaType::String);
		return lua_tostring(ls, idx);
	}
	LuaType LCV<const char*>::operator()(const char* c) const {
		if(c)
			return LuaType::String;
		return LuaType::Nil;
	}
	DEF_LCV_OSTREAM(const char*)

	// [LCV<std::string> = LUA_TSTRING]
	void LCV<std::string>::operator()(lua_State* ls, const std::string& s) const {
		lua_pushlstring(ls, s.c_str(), s.length());
	}
	std::string LCV<std::string>::operator()(const int idx, lua_State* ls, LPointerSP* /*spm*/) const {
		if(lua_type(ls, idx) == LUA_TNIL)
			return std::string();
		LuaState::CheckType(ls, idx, LuaType::String);
		std::size_t len;
		const char* c =lua_tolstring(ls, idx, &len);
		return std::string(c, len);
	}
	LuaType LCV<std::string>::operator()(const std::string&) const {
		return LuaType::String;
	}
	DEF_LCV_OSTREAM(std::string)

	namespace {
		bool IsRadian(lua_State* ls, const int idx) {
			LuaState lsc(ls, true);
			lsc.getField(idx, luaNS::Postfix);
			auto cname = lsc.toString(-1);
			lsc.pop();
			if(cname == luaNS::postfix::Radian)
				return true;
			D_Assert(cname==luaNS::postfix::Degree, "invalid angle type (required Degree or Radian, but got %d", cname.c_str());
			return false;
		}
	}
	// [LCV<frea::DegF>]
	void LCV<frea::DegF>::operator()(lua_State* ls, const frea::DegF& d) const {
		LCV_In<frea::DegF>()(ls, d);
	}
	frea::DegF LCV<frea::DegF>::operator()(const int idx, lua_State* ls, LPointerSP* spm) const {
		if(IsRadian(ls, idx)) {
			// Degreeに変換して返す
			const auto rad = LCV_In<frea::RadF>()(idx, ls, spm);
			return frea::DegF(rad);
		} else {
			return LCV_In<frea::DegF>()(idx, ls, spm);
		}
	}
	LuaType LCV<frea::DegF>::operator()(const frea::DegF&) const {
		return LuaType::Table;
	}
	DEF_LCV_OSTREAM_PAIR(frea::DegF, Degree)

	// [LCV<frea::RadF>]
	void LCV<frea::RadF>::operator()(lua_State* ls, const frea::RadF& d) const {
		LCV_In<frea::RadF>()(ls, d);
	}
	frea::RadF LCV<frea::RadF>::operator()(const int idx, lua_State* ls, LPointerSP* spm) const {
		if(!IsRadian(ls, idx)) {
			// Radianに変換して返す
			const auto rad = LCV_In<frea::DegF>()(idx, ls, spm);
			return frea::RadF(rad);
		} else {
			return LCV_In<frea::RadF>()(idx, ls, spm);
		}
	}
	LuaType LCV<frea::RadF>::operator()(const frea::RadF&) const {
		return LuaType::Table;
	}
	DEF_LCV_OSTREAM_PAIR(frea::RadF, Radian)

	// [LCV<lua_Integer> = LUA_TNUMBER]
	void LCV<lua_Integer>::operator()(lua_State* ls, const lua_Integer i) const {
		lua_pushinteger(ls, i);
	}
	lua_Integer LCV<lua_Integer>::operator()(const int idx, lua_State* ls, LPointerSP* /*spm*/) const {
		LuaState::CheckType(ls, idx, LuaType::Number);
		return lua_tointeger(ls, idx);
	}
	LuaType LCV<lua_Integer>::operator()(lua_Integer) const {
		return LuaType::Number;
	}
	DEF_LCV_OSTREAM(lua_Integer)

	// --- LCV<lua_Number> = LUA_TNUMBER
	void LCV<lua_Number>::operator()(lua_State* ls, const lua_Number f) const {
		lua_pushnumber(ls, f);
	}
	lua_Number LCV<lua_Number>::operator()(const int idx, lua_State* ls, LPointerSP* /*spm*/) const {
		LuaState::CheckType(ls, idx, LuaType::Number);
		return lua_tonumber(ls, idx);
	}
	LuaType LCV<lua_Number>::operator()(lua_Number) const {
		return LuaType::Number;
	}
	DEF_LCV_OSTREAM(lua_Number)

	// [LCV<lua_State*> = LUA_TTHREAD]
	void LCV<lua_State*>::operator()(lua_State* ls, lua_State* lsp) const {
		LCV<Lua_SP>()(ls, LuaState::GetLS_SP(lsp));
	}
	lua_State* LCV<lua_State*>::operator()(const int idx, lua_State* ls, LPointerSP* spm) const {
		if(auto sp = LCV<Lua_SP>()(idx, ls, spm))
			return sp->getLS();
		return nullptr;
	}
	LuaType LCV<lua_State*>::operator()(lua_State*) const {
		return LuaType::Thread;
	}
	DEF_LCV_OSTREAM(lua_State*)

	// [LCV<lubee::RectF> = LUA_TTABLE]
	void LCV<lubee::RectF>::operator()(lua_State* ls, const lubee::RectF& r) const {
		lua_createtable(ls, 4, 0);
		int idx = 1;
		const auto fnSet = [ls, &idx](const float f){
			lua_pushinteger(ls, idx++);
			lua_pushnumber(ls, f);
			lua_settable(ls, -3);
		};
		fnSet(r.x0);
		fnSet(r.x1);
		fnSet(r.y0);
		fnSet(r.y1);
	}
	lubee::RectF LCV<lubee::RectF>::operator()(const int idx, lua_State* ls, LPointerSP*) const {
		const auto fnGet = [ls, idx](const int n){
			lua_pushinteger(ls, n);
			lua_gettable(ls, idx);
			auto ret = lua_tonumber(ls, -1);
			lua_pop(ls, 1);
			return ret;
		};
		return {fnGet(1), fnGet(2), fnGet(3), fnGet(4)};
	}
	LuaType LCV<lubee::RectF>::operator()(const lubee::RectF&) const {
		return LuaType::Table;
	}
	DEF_LCV_OSTREAM_PAIR(lubee::RectF, Rect)

	// [LCV<lubee::SizeF> = LUA_TTABLE]
	void LCV<lubee::SizeF>::operator()(lua_State* ls, const lubee::SizeF& s) const {
		lua_createtable(ls, 2, 0);
		lua_pushinteger(ls, 1);
		lua_pushnumber(ls, s.width);
		lua_settable(ls, -3);
		lua_pushinteger(ls, 2);
		lua_pushnumber(ls, s.height);
		lua_settable(ls, -3);
	}
	lubee::SizeF LCV<lubee::SizeF>::operator()(const int idx, lua_State* ls, LPointerSP*) const {
		const auto fnGet = [ls, idx](const int n){
			lua_pushinteger(ls, n);
			lua_gettable(ls, idx);
			auto ret = lua_tonumber(ls, -1);
			lua_pop(ls, 1);
			return ret;
		};
		return {fnGet(1), fnGet(2)};
	}
	LuaType LCV<lubee::SizeF>::operator()(const lubee::SizeF&) const {
		return LuaType::Table;
	}
	DEF_LCV_OSTREAM_PAIR(lubee::SizeF, Size)

	// [LCV<Lua_SP> = LUA_TTHREAD]
	void LCV<Lua_SP>::operator()(lua_State* ls, const Lua_SP& sp) const {
		sp->_registerLua();
		sp->pushSelf();
		lua_xmove(sp->getLS(), ls, 1);
	}
	Lua_SP LCV<Lua_SP>::operator()(const int idx, lua_State* ls, LPointerSP* /*spm*/) const {
		const auto typ = lua_type(ls, idx);
		if(typ == LUA_TTHREAD)
			return LuaState::GetLS_SP(lua_tothread(ls, idx));
		if(typ == LUA_TNIL || typ == LUA_TNONE) {
			// 自身を返す
			return LuaState::GetLS_SP(ls);
		}
		LuaState::CheckType(ls, idx, LuaType::Thread);
		return Lua_SP();
	}
	LuaType LCV<Lua_SP>::operator()(const Lua_SP&) const {
		return LuaType::Thread;
	}
	DEF_LCV_OSTREAM(Lua_SP)

	// [LCV<void*> = LUA_TLIGHTUSERDATA]
	void LCV<void*>::operator()(lua_State* ls, const void* ud) const {
		lua_pushlightuserdata(ls, const_cast<void*>(ud));
	}
	void* LCV<void*>::operator()(const int idx, lua_State* ls, LPointerSP* /*spm*/) const {
		try {
			LuaState::CheckType(ls, idx, LuaType::Userdata);
		} catch(const LuaState::EType& e) {
			LuaState::CheckType(ls, idx, LuaType::LightUserdata);
		}
		return lua_touserdata(ls, idx);
	}
	LuaType LCV<void*>::operator()(const void*) const {
		return LuaType::LightUserdata;
	}
	DEF_LCV_OSTREAM(void*)

	// [LCV<lua_CFunction> = LUA_TFUNCTION]
	void LCV<lua_CFunction>::operator()(lua_State* ls, const lua_CFunction f) const {
		lua_pushcclosure(ls, f, 0);
	}
	lua_CFunction LCV<lua_CFunction>::operator()(const int idx, lua_State* ls, LPointerSP* /*spm*/) const {
		LuaState::CheckType(ls, idx, LuaType::Function);
		return lua_tocfunction(ls, idx);
	}
	LuaType LCV<lua_CFunction>::operator()(lua_CFunction) const {
		return LuaType::Function;
	}
	DEF_LCV_OSTREAM(lua_CFunction)

	// [LCV<Timepoint> = LUA_TNUMBER]
	void LCV<Timepoint>::operator()(lua_State* ls, const Timepoint& t) const {
		LCV<Duration> dur;
		dur(ls, Duration(t.time_since_epoch()));
	}
	Timepoint LCV<Timepoint>::operator()(const int idx, lua_State* ls, LPointerSP* spm) const {
		LCV<Duration> dur;
		return Timepoint(dur(idx, ls, spm));
	}
	LuaType LCV<Timepoint>::operator()(const Timepoint&) const {
		return LuaType::Number;
	}
	DEF_LCV_OSTREAM(Timepoint)

	// [LCV<LCTable_SP> = LUA_TTABLE]
	void LCV<LCTable_SP>::operator()(lua_State* ls, const LCTable_SP& t) const {
        LuaState lsc(ls, false);
		lsc.newTable(0, t->size());
		for(auto& ent : *t) {
			lsc.setField(-1, ent.first, ent.second);
		}
	}
	LCTable_SP LCV<LCTable_SP>::operator()(int idx, lua_State* ls, LPointerSP* spm) const {
		LuaState::CheckType(ls, idx, LuaType::Table);

		spi::Optional<LPointerSP> opSet;
		if(!spm) {
			opSet = spi::construct();
			spm = &(*opSet);
		}
		LuaState lsc(ls, true);
		const void* ptr = lsc.toPointer(idx);
		auto itr = spm->find(ptr);
		if(itr != spm->end())
			return boost::get<LCTable_SP>(itr->second);

		// 循環参照対策で先にエントリを作っておく
		LCTable_SP ret(new LCTable());
		TryEmplace(*spm, ptr, ret);
		idx = lsc.absIndex(idx);
		lsc.push(LuaNil());
		while(lsc.next(idx) != 0) {
			lsc.pushValue(-2);
			// key=-1 value=-2
			TryEmplace(*ret, lsc.toLCValue(-1, spm), lsc.toLCValue(-2, spm));
			// valueは取り除きkeyはlua_nextのために保持
			lsc.pop(2);
		}
		return ret;
	}
	LuaType LCV<LCTable_SP>::operator()(const LCTable_SP&) const {
		return LuaType::Table;
	}

	// [LCV<LCValue>]
	namespace {
		const std::function<LCValue (lua_State* ls, int idx, LPointerSP* spm)> c_toLCValue[LUA_NUMTAGS] = {
			// LUA_TNIL
			[](lua_State* /*ls*/, int /*idx*/, LPointerSP* /*spm*/){ return LCValue(LuaNil()); },
			// LUA_TBOOLEAN
			[](lua_State* ls, const int idx, LPointerSP* spm){ return LCValue(LCV<bool>()(idx,ls,spm)); },
			// LUA_TLIGHTUSERDATA
			[](lua_State* ls, const int idx, LPointerSP* spm){ return LCValue(LCV<void*>()(idx,ls,spm)); },
			// LUA_TNUMBER
			[](lua_State* ls, const int idx, LPointerSP* spm){ return LCValue(LCV<lua_Number>()(idx,ls,spm)); },
			// LUA_TSTRING
			[](lua_State* ls, const int idx, LPointerSP* spm){ return LCValue(LCV<const char*>()(idx,ls,spm)); },
			// LUA_TTABLE
			[](lua_State* ls, const int idx, LPointerSP* spm){ return LCValue(LCV<LCTable_SP>()(idx,ls,spm)); },
			// LUA_TFUNCTION
			[](lua_State* ls, const int idx, LPointerSP* spm){ return LCValue(LCV<lua_CFunction>()(idx,ls,spm)); },
			// LUA_TUSERDATA
			[](lua_State* ls, const int idx, LPointerSP* spm){ return LCValue(LCV<void*>()(idx,ls,spm)); },
			// LUA_TTHREAD
			[](lua_State* ls, const int idx, LPointerSP* spm){ return LCValue(LCV<Lua_SP>()(idx,ls,spm)); }
		};
	}
	void LCV<LCValue>::operator()(lua_State* ls, const LCValue& lcv) const {
		lcv.push(ls);
	}
	LCValue LCV<LCValue>::operator()(const int idx, lua_State* ls, LPointerSP* spm) const {
		const CheckTop ct(ls);
		const auto typ = lua_type(ls, idx);
		// Tableにおいて、_postfixフィールド値がVならば_sizeフィールドを読み込みVecTに変換
		if(typ == LUA_TTABLE) {
			lua_pushvalue(ls, idx);
			LValueS lvs(ls);
			LValueS postfix = lvs[luaNS::Postfix];
			if(postfix.type() == LuaType::String) {
				const std::string pf(postfix.toString());
				const auto retAs = [&lvs](auto* p){
					void* ptr = LValueS(lvs[luaNS::objBase::Pointer]).toUserData();
					return *static_cast<decltype(p)>(ptr);
				};
				if(pf == luaNS::postfix::Vector) {
					const int size = LValueS(lvs[luaNS::vector::Size]).toInteger();
					switch(size) {
						case 2:
							return retAs((Vec2*)nullptr);
						case 3:
							return retAs((Vec3*)nullptr);
						case 4:
							return retAs((Vec4*)nullptr);
					}
					AssertF("invalid vector size (%d)", size);
				} else if(pf == luaNS::postfix::Quat) {
					return retAs((frea::Quat*)nullptr);
				} else if(pf == luaNS::postfix::Degree) {
					return retAs((frea::DegF*)nullptr);
				} else if(pf == luaNS::postfix::Radian) {
					return retAs((frea::RadF*)nullptr);
				} else if(pf == luaNS::postfix::ExpQuat) {
					return retAs((frea::ExpQuat*)nullptr);
				} else if(pf == luaNS::postfix::Matrix) {
					const int size = LValueS(lvs[luaNS::matrix::Size]).toInteger();
					switch(size) {
						case 2:
							return retAs((Mat2*)nullptr);
						case 3:
							return retAs((Mat3*)nullptr);
						case 4:
							return retAs((Mat4*)nullptr);
					}
					AssertF("invalid matrix size (%d)", size);
				} else {
					AssertF("not supported.");
				}
			}
		}
		D_Assert0(typ < int(countof(c_toLCValue)));
		return c_toLCValue[typ](ls, idx, spm);
	}
	LuaType LCV<LCValue>::operator()(const LCValue& c) const {
		return c.type();
	}
	DEF_LCV_OSTREAM(LCValue)

	// [LCV<LValueS>]
	void LCV<LValueS>::operator()(lua_State* ls, const LValueS& t) const {
		t.prepareValue(ls);
	}
	LValueS LCV<LValueS>::operator()(const int idx, lua_State* ls, LPointerSP* /*spm*/) const {
		lua_pushvalue(ls, idx);
		return LValueS(ls);
	}
	LuaType LCV<LValueS>::operator()(const LValueS& t) const {
		return t.type();
	}
	DEF_LCV_OSTREAM(LValueS)

	// [LCV<LValueG>]
	void LCV<LValueG>::operator()(lua_State* ls, const LValueG& t) const {
		t.prepareValue(ls);
	}
	LValueG LCV<LValueG>::operator()(const int idx, lua_State* ls, LPointerSP* /*spm*/) const {
		lua_pushvalue(ls, idx);
		Lua_SP sp = LuaState::GetMainLS_SP(ls);
		return LValueG(sp);
	}
	LuaType LCV<LValueG>::operator()(const LValueG& t) const {
		return t.type();
	}
	DEF_LCV_OSTREAM(LValueG)

	namespace {
		// key=void*, value=table{"udata" = (shared|weak)pointer}
		// idxにある値がnilなら空ポインタを、tableならudataエントリの(shared|weak)ポインタを取り出す
		template <class P>
		auto LoadSmartPtr(int idx, lua_State* ls, const std::string& func) {
			const RewindTop rt(ls);
			LuaState lsc(ls, false);
			idx = lsc.absIndex(idx);
			lsc.getGlobal(func);
			lsc.pushValue(idx);
			lsc.call(1, 1);
			if(lsc.type(-1) == LuaType::Userdata)
				return *reinterpret_cast<P*>(lsc.toUserData(-1));
			return P();
		}
		auto LoadVoidWP(const int idx, lua_State* ls) {
			return LoadSmartPtr<void_wp>(idx, ls, luaNS::GetWP);
		}
		template <class T>
		auto GetPointer(const std::shared_ptr<T>& p) {
			return p.get();
		}
		template <class T>
		auto GetPointer(const std::weak_ptr<T>& p) {
			return p.lock().get();
		}
		template <class P>
		bool PushSmartPtr(lua_State* ls, const std::string& check, const std::string& make, const std::string& name, const P& p) {
			LuaState lsc(ls, false);
			void* ptr = GetPointer(p);
			lsc.getGlobal(check);
			lsc.push(ptr);
			lsc.call(1, 1);
			if(lsc.type(-1) == LuaType::Nil) {
				lsc.pop(1);
				lsc.getGlobal(make);
				lsc.push(ptr);
				lsc.push(name);
				MakeUserdataWithDtor(lsc, p);
				lsc.call(3, 1);
				return true;
			}
			return false;
		}
	}
	void_sp LoadVoidSP(const int idx, lua_State* ls) {
		return LoadSmartPtr<void_sp>(idx, ls, luaNS::GetSP);
	}
	bool PushSP(lua_State* ls, const std::string& name, const void_sp& sp) {
		if(!sp) {
			// nullハンドルの場合はnilをpushする
			LCV<LuaNil>()(ls, LuaNil());
			return false;
		} else
			return PushSmartPtr(ls, luaNS::HasSP, luaNS::MakeSP, name, sp);
	}
	bool PushWP(lua_State* ls, const std::string& name, const void_wp& wp) {
		if(auto sp = wp.lock()) {
			return PushSmartPtr(ls, luaNS::HasWP, luaNS::MakeWP, name, wp);
		} else {
			// nullハンドルの場合はnilをpushする
			LCV<LuaNil>()(ls, LuaNil());
			return false;
		}
	}
	// [LCV<void_sp>]
	void LCV<void_sp>::operator()(lua_State* ls, const void_sp& p) const {
		PushSP(ls, luaNS::Void, p);
	}
	void_sp LCV<void_sp>::operator()(const int idx, lua_State* ls, LPointerSP* /*spm*/) const {
		return LoadVoidSP(idx, ls);
	}
	LuaType LCV<void_sp>::operator()(const void_sp&) const {
		return LuaType::Table;
	}
	DEF_LCV_OSTREAM(void_sp)

	// [LCV<void_wp>]
	void LCV<void_wp>::operator()(lua_State* ls, const void_wp& w) const {
		PushWP(ls, luaNS::Void, w);
	}
	void_wp LCV<void_wp>::operator()(const int idx, lua_State* ls, LPointerSP* /*spm*/) const {
		return LoadVoidWP(idx, ls);
	}
	LuaType LCV<void_wp>::operator()(const void_wp&) const {
		return LuaType::Table;
	}
	DEF_LCV_OSTREAM(void_wp)
}
