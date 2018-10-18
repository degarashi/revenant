#pragma once
#include <tuple>

namespace rev {
	namespace luaNS {
		extern const std::string Void;
	}
	class LuaState;
	namespace lua {
		template <
			class Mgr, class C, class Getter,
			class...Member, class... Method, class Maker
		>
		void LuaExportImpl(
			LuaState& lsc,
			const std::tuple<Member...>& member,
			const std::tuple<Method...>& method,
			Maker
		);
		template <class T>
		void LuaExport(LuaState& lsc, T*);
		template <class T>
		const char* LuaName(T*) { return luaNS::Void.c_str(); }
		template <class T>
		struct LuaDefine : std::false_type {};
	}
}

#define NOTHING
#define DEF_LUAIMPORT(clazz) \
	namespace rev { \
		namespace lua { \
			template <> \
			const char* LuaName(clazz*); \
			template <> \
			void LuaExport(LuaState&, clazz*); \
			template <> \
			struct LuaDefine<clazz> : std::true_type {}; \
		} \
	}
