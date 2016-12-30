#pragma once
#include "spine/enum.hpp"
#include <memory>
#include <lua.hpp>

namespace rev {
	namespace luaNS {
		extern const std::string HasSP,
								MakeSP,
								GetSP,
								HasWP,
								MakeWP,
								GetWP,
								Void;
		extern const std::string ScriptResourceEntry,
								SystemScriptResourceEntry,
								ScriptExtension;
		extern const std::string ToString;
		extern const std::string SetState,
								SwitchState,
								Null,
								Object,
								ConstructPtr,
								DefineObject,
								MakeFSMachine,
								FSMachine,
								MakePreENV,
								Ctor,
								RecvMsg,
								RecvMsgCpp,
								OnUpdate,
								OnPause,
								OnEffectReset,
								OnResume,
								OnExit,
								System,
								Global,
								Postfix;
		namespace objBase {
			extern const std::string ValueR,
									ValueW,
									Func,
									Metatable,
									Base,
									Name,
									Pointer,
									Udata,
									New;
			namespace valueR {
				extern const std::string NumRef;
			}
		}
		namespace postfix {
			extern const std::string Vector,
									Matrix,
									Quat,
									ExpQuat,
									Plane,
									Degree,
									Radian;
		}
		namespace vector {
			extern const std::string Size;
		}
		namespace matrix {
			extern const std::string Size;
		}
		namespace system {
			extern const std::string PathSeparation,
									PathReplaceMark,
									Package,
									Path;
		}
	}
	struct LuaNil {
		bool operator == (LuaNil) const;
		bool operator != (LuaNil) const;
	};
	std::ostream& operator << (std::ostream& os, LuaNil);

	DefineEnumPair(LuaType,
		((Nil)(LUA_TNIL))
		((Number)(LUA_TNUMBER))
		((Boolean)(LUA_TBOOLEAN))
		((String)(LUA_TSTRING))
		((Table)(LUA_TTABLE))
		((Function)(LUA_TFUNCTION))
		((Userdata)(LUA_TUSERDATA))
		((Thread)(LUA_TTHREAD))
		((LightUserdata)(LUA_TLIGHTUSERDATA))
	);
	class LuaState;
	using Lua_SP = std::shared_ptr<LuaState>;
	using Lua_WP = std::weak_ptr<LuaState>;
	class LCTable;
	using LCTable_SP = std::shared_ptr<LCTable>;
	using ILua_SP = std::shared_ptr<lua_State>;

	class LCValue;
	template <class T>
	class LValue;
	class LV_Global;
	using LValueG = LValue<LV_Global>;
	class LV_Stack;
	using LValueS = LValue<LV_Stack>;

	namespace detail {
		double LuaOtherNumber(std::integral_constant<int,4>);
		float LuaOtherNumber(std::integral_constant<int,8>);
		int32_t LuaOtherInteger(std::integral_constant<int,8>);
		int64_t LuaOtherInteger(std::integral_constant<int,4>);
	}
	//! lua_Numberがfloatならdouble、doubleならfloat
	using lua_OtherNumber = decltype(detail::LuaOtherNumber(std::integral_constant<int,sizeof(lua_Number)>()));
	//! lua_Integerがint32_tならint64_t、int64_tならint32_t
	using lua_OtherInteger = decltype(detail::LuaOtherInteger(std::integral_constant<int,sizeof(lua_Integer)>()));
	using lua_IntegerU = std::make_unsigned<lua_Integer>::type;
	using lua_OtherIntegerU = std::make_unsigned<lua_OtherInteger>::type;
}
namespace std {
	template <>
	struct hash<rev::LuaNil> {
		std::size_t operator()(rev::LuaNil) const {
			return 0;
		}
	};
	template <class T>
	struct hash<std::weak_ptr<T>> {
		std::size_t operator()(const std::weak_ptr<T>& p) const {
			auto sp = p.lock();
			return std::hash<decltype(sp)>()(sp);
		}
	};
}
