#pragma once
#include "lv_common.hpp"

namespace rev {
	class LuaState;
	namespace lua {
		template <class T>
		void LuaExport(LuaState& lsc, T*);
		template <class T>
		const char* LuaName(T*) { return luaNS::Void.c_str(); }
		template <class T>
		struct LuaDefine : std::false_type {};
	}
}

#define DEF_LUAIMPORT(clazz) \
	namespace rev { \
		namespace lua { \
			template <> \
			const char* LuaName(clazz*); \
			template <> \
			void LuaExport(LuaState& lsc, clazz*); \
			template <> \
			struct LuaDefine<clazz> : std::true_type {}; \
		} \
	}
