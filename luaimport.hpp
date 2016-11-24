#pragma once

#define DEF_LUAIMPORT_BASE \
	namespace rev { \
		class LuaState; \
		namespace lua { \
			template <class T> \
			void LuaExport(LuaState& lsc, T*); \
			template <class T> \
			const char* LuaName(T*); \
		} \
	}
#define DEF_LUAIMPORT(clazz) \
	DEF_LUAIMPORT_BASE \
	namespace rev { \
		namespace lua { \
			template <> \
			const char* LuaName(clazz*); \
			template <> \
			void LuaExport(LuaState& lsc, clazz*); \
		} \
	}
