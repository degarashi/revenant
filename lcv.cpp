#include "lubee/rect.hpp"
#include "emplace.hpp"
#include "lvalue.hpp"
#include "rewindtop.hpp"

namespace rev {
	DEF_LCV_OSTREAM(void)
	DEF_LCV_OSTREAM(lua_OtherNumber)
	DEF_LCV_OSTREAM(lua_OtherInteger)
	DEF_LCV_OSTREAM(lua_OtherIntegerU)

	// [LCV<LuaNil> = LUA_TNIL]
	int LCV<LuaNil>::operator()(lua_State* ls, LuaNil) const {
		lua_pushnil(ls);
		return 1;
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
	int LCV<bool>::operator()(lua_State* ls, const bool b) const {
		lua_pushboolean(ls, b);
		return 1;
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
	int LCV<const char*>::operator()(lua_State* ls, const char* c) const {
		lua_pushstring(ls, c);
		return 1;
	}
	const char* LCV<const char*>::operator()(const int idx, lua_State* ls, LPointerSP* /*spm*/) const {
		LuaState::CheckType(ls, idx, LuaType::String);
		return lua_tostring(ls, idx);
	}
	LuaType LCV<const char*>::operator()(const char*) const {
		return LuaType::String;
	}
	DEF_LCV_OSTREAM(const char*)

	// [LCV<std::string> = LUA_TSTRING]
	int LCV<std::string>::operator()(lua_State* ls, const std::string& s) const {
		lua_pushlstring(ls, s.c_str(), s.length());
		return 1;
	}
	std::string LCV<std::string>::operator()(const int idx, lua_State* ls, LPointerSP* /*spm*/) const {
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
		auto GetAngleType(lua_State* ls, const int idx) {
			LuaState lsc(ls, true);
			lsc.getField(idx, luaNS::objBase::ClassName);
			auto cname = lsc.toString(-1);
			lsc.pop();
			return cname;
		}
	}
	// [LCV<frea::DegF>]
	int LCV<frea::DegF>::operator()(lua_State* ls, const frea::DegF& d) const {
		return LCV_In<frea::DegF>()(ls, d);
	}
	frea::DegF LCV<frea::DegF>::operator()(const int idx, lua_State* ls, LPointerSP* spm) const {
		auto cname = GetAngleType(ls, idx);
		if(cname == "Radian") {
			// Degreeに変換して返す
			auto rad = LCV_In<frea::RadF>()(idx, ls, spm);
			return frea::DegF(rad);
		} else {
			Assert(cname=="Degree", "invalid angle type (required Degree or Radian, but got %d", cname.c_str());
			return LCV_In<frea::DegF>()(idx, ls, spm);
		}
	}
	LuaType LCV<frea::DegF>::operator()(const frea::DegF&) const {
		return LuaType::Userdata;
	}
	DEF_LCV_OSTREAM_PAIR(frea::DegF, Degree)

	// [LCV<frea::RadF>]
	int LCV<frea::RadF>::operator()(lua_State* ls, const frea::RadF& d) const {
		return LCV_In<frea::RadF>()(ls, d);
	}
	frea::RadF LCV<frea::RadF>::operator()(const int idx, lua_State* ls, LPointerSP* spm) const {
		auto cname = GetAngleType(ls, idx);
		if(cname == "Degree") {
			// Radianに変換して返す
			auto rad = LCV_In<frea::DegF>()(idx, ls, spm);
			return frea::RadF(rad);
		} else {
			Assert(cname=="Radian", "invalid angle type (required Degree or Radian, but got %d", cname.c_str());
			return LCV_In<frea::RadF>()(idx, ls, spm);
		}
	}
	LuaType LCV<frea::RadF>::operator()(const frea::RadF&) const {
		return LuaType::Userdata;
	}
	DEF_LCV_OSTREAM_PAIR(frea::RadF, Radian)

	// [LCV<lua_Integer> = LUA_TNUMBER]
	int LCV<lua_Integer>::operator()(lua_State* ls, const lua_Integer i) const {
		lua_pushinteger(ls, i);
		return 1;
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
	int LCV<lua_Number>::operator()(lua_State* ls, const lua_Number f) const {
		lua_pushnumber(ls, f);
		return 1;
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
	int LCV<lua_State*>::operator()(lua_State* ls, lua_State* lsp) const {
		return LCV<Lua_SP>()(ls, LuaState::GetLS_SP(lsp));
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
	int LCV<lubee::RectF>::operator()(lua_State* ls, const lubee::RectF& r) const {
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
		return 1;
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
	int LCV<lubee::SizeF>::operator()(lua_State* ls, const lubee::SizeF& s) const {
		lua_createtable(ls, 2, 0);
		lua_pushinteger(ls, 1);
		lua_pushnumber(ls, s.width);
		lua_settable(ls, -3);
		lua_pushinteger(ls, 2);
		lua_pushnumber(ls, s.height);
		lua_settable(ls, -3);
		return 1;
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
	int LCV<Lua_SP>::operator()(lua_State* ls, const Lua_SP& sp) const {
		sp->_registerLua();
		sp->pushSelf();
		lua_xmove(sp->getLS(), ls, 1);
		return 1;
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
	int LCV<void*>::operator()(lua_State* ls, const void* ud) const {
		lua_pushlightuserdata(ls, const_cast<void*>(ud));
		return 1;
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
	int LCV<lua_CFunction>::operator()(lua_State* ls, const lua_CFunction f) const {
		lua_pushcclosure(ls, f, 0);
		return 1;
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
	int LCV<Timepoint>::operator()(lua_State* ls, const Timepoint& t) const {
		LCV<Duration> dur;
		dur(ls, Duration(t.time_since_epoch()));
		return 1;
	}
	Timepoint LCV<Timepoint>::operator()(const int idx, lua_State* ls, LPointerSP* /*spm*/) const {
		LCV<Duration> dur;
		return Timepoint(dur(idx, ls));
	}
	LuaType LCV<Timepoint>::operator()(const Timepoint&) const {
		return LuaType::Number;
	}
	DEF_LCV_OSTREAM(Timepoint)

	// [LCV<LCTable_SP> = LUA_TTABLE]
	int LCV<LCTable_SP>::operator()(lua_State* ls, const LCTable_SP& t) const {
        LuaState lsc(ls, false);
		lsc.newTable(0, t->size());
		for(auto& ent : *t) {
			lsc.setField(-1, ent.first, ent.second);
		}
		return 1;
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
	int LCV<LCValue>::operator()(lua_State* ls, const LCValue& lcv) const {
		lcv.push(ls);
		return 1;
	}
	LCValue LCV<LCValue>::operator()(const int idx, lua_State* ls, LPointerSP* spm) const {
		const CheckTop ct(ls);
		const auto typ = lua_type(ls, idx);
		// Tableにおいて、_prefixフィールド値がVならば_sizeフィールドを読み込みVecTに変換
		if(typ == LUA_TTABLE) {
			lua_pushvalue(ls, idx);
			LValueS lvs(ls);
			LValueS postfix = lvs["_postfix"];
			if(postfix.type() == LuaType::String &&
				std::string(postfix.toString()) == "V")
			{
				const int size = LValueS(lvs["_size"]).toInteger();
				const void* ptr = LValueS(lvs["pointer"]).toUserData();
				switch(size) {
					case 2:
						return *static_cast<const frea::Vec2*>(ptr);
					case 3:
						return *static_cast<const frea::Vec3*>(ptr);
					case 4:
						return *static_cast<const frea::Vec4*>(ptr);
				}
				Assert(false, "invalid vector size (%d)", size);
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
	int LCV<LValueS>::operator()(lua_State* ls, const LValueS& t) const {
		t.prepareValue(ls);
		return 1;
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
	int LCV<LValueG>::operator()(lua_State* ls, const LValueG& t) const {
		t.prepareValue(ls);
		return 1;
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

	// [LCV<frea::Vec2>]
	int LCV<frea::Vec2>::operator()(lua_State*, const frea::Vec2&) const {
		return 0;
	}
	frea::Vec2 LCV<frea::Vec2>::operator()(const int, lua_State*, LPointerSP* /*spm*/) const {
		return {};
	}
	LuaType LCV<frea::Vec2>::operator()(const frea::Vec2&) const {
		return LuaType::Nil;
	}
	DEF_LCV_OSTREAM(frea::Vec2)

	// [LCV<frea::Vec3>]
	int LCV<frea::Vec3>::operator()(lua_State*, const frea::Vec3&) const {
		return 0;
	}
	frea::Vec3 LCV<frea::Vec3>::operator()(const int, lua_State*, LPointerSP* /*spm*/) const {
		return {};
	}
	LuaType LCV<frea::Vec3>::operator()(const frea::Vec3&) const {
		return LuaType::Nil;
	}
	DEF_LCV_OSTREAM(frea::Vec3)

	// [LCV<frea::Vec4>]
	int LCV<frea::Vec4>::operator()(lua_State*, const frea::Vec4&) const {
		return 0;
	}
	frea::Vec4 LCV<frea::Vec4>::operator()(const int, lua_State*, LPointerSP* /*spm*/) const {
		return {};
	}
	LuaType LCV<frea::Vec4>::operator()(const frea::Vec4&) const {
		return LuaType::Nil;
	}
	DEF_LCV_OSTREAM(frea::Vec4)

	// [LCV<frea::Mat2>]
	int LCV<frea::Mat2>::operator()(lua_State*, const frea::Mat2&) const {
		return 0;
	}
	frea::Mat2 LCV<frea::Mat2>::operator()(const int, lua_State*, LPointerSP* /*spm*/) const {
		return {};
	}
	LuaType LCV<frea::Mat2>::operator()(const frea::Mat2&) const {
		return LuaType::Nil;
	}
	DEF_LCV_OSTREAM(frea::Mat2)

	// [LCV<frea::Mat3>]
	int LCV<frea::Mat3>::operator()(lua_State*, const frea::Mat3&) const {
		return 0;
	}
	frea::Mat3 LCV<frea::Mat3>::operator()(const int, lua_State*, LPointerSP* /*spm*/) const {
		return {};
	}
	LuaType LCV<frea::Mat3>::operator()(const frea::Mat3&) const {
		return LuaType::Nil;
	}
	DEF_LCV_OSTREAM(frea::Mat3)

	// [LCV<frea::Mat4>]
	int LCV<frea::Mat4>::operator()(lua_State*, const frea::Mat4&) const {
		return 0;
	}
	frea::Mat4 LCV<frea::Mat4>::operator()(const int, lua_State*, LPointerSP* /*spm*/) const {
		return {};
	}
	LuaType LCV<frea::Mat4>::operator()(const frea::Mat4&) const {
		return LuaType::Nil;
	}
	DEF_LCV_OSTREAM(frea::Mat4)

	// [LCV<frea::Quat>]
	int LCV<frea::Quat>::operator()(lua_State*, const frea::Quat&) const {
		return 0;
	}
	frea::Quat LCV<frea::Quat>::operator()(const int, lua_State*, LPointerSP* /*spm*/) const {
		return {};
	}
	LuaType LCV<frea::Quat>::operator()(const frea::Quat&) const {
		return LuaType::Nil;
	}
	DEF_LCV_OSTREAM(frea::Quat)
}
