#pragma once
#include "lcv.hpp"
#include "lubee/src/meta/check_macro.hpp"
#include "lubee/src/meta/enable_if.hpp"
#include "lubee/src/error.hpp"
#include <boost/preprocessor.hpp>

namespace rev {
	DEF_HASMETHOD(LuaExport)
	class LuaState;
	// LuaExportというメソッドを持っていればそれを呼ぶ
	template <class T, ENABLE_IF(HasMethod_LuaExport_t<T>{})>
	void CallLuaExport(LuaState& lsc) {
		T::LuaExport(lsc);
	}
	template <class T, ENABLE_IF(!HasMethod_LuaExport_t<T>{})>
	void CallLuaExport(LuaState&) {}
}
#define DEF_LUAIMPLEMENT_DERIVED(clazz, base) \
	namespace rev { \
		namespace lua { \
			template <> \
			const char* LuaName(clazz*) { return LuaName((base*)nullptr); } \
			template <> \
			void LuaExport(LuaState& lsc, clazz*) { LuaExport(lsc, (base*)nullptr); } \
		} \
	}

namespace rev {
	template <class Dummy_t, class C, class... Ts>
	int MakeObject(lua_State* ls) {
		FuncCall<Ts...>::callCB(
			[ls](auto&&... args) {
				LCV<C>()(ls, C{std::forward<decltype(args)>(args)...});
			},
			ls,
			static_cast<int>(-sizeof...(Ts))
		);
		return 1;
	}
	template <class Dummy_t, class C, class... Ts>
	int MakeSPObject(lua_State* ls) {
		auto obj = FuncCall<Ts...>::callCB(
			[](auto&&... args) {
				return std::make_shared<C>(std::forward<decltype(args)>(args)...);
			},
			ls,
			static_cast<int>(-sizeof...(Ts))
		);
		LCV<decltype(obj)>()(ls, obj);
		return 1;
	}
	//! (LuaのClassT::New()から呼ばれる)オブジェクトのリソースハンドルを作成
	template <class Mgr_t, class TF, class... Ts>
	int MakeHandle(lua_State* ls) {
		auto obj = FuncCall<Ts...>::callCB(
			[](auto&&... args) {
				return Mgr_t::ref().template emplace<TF>(std::forward<decltype(args)>(args)...);
			},
			ls,
			static_cast<int>(-sizeof...(Ts))
		);
		LCV<decltype(obj)>()(ls, obj);
		return 1;
	}
	//! デフォルトコンストラクタを持たないオブジェクト用
	template <class... Ts>
	int MakeFake(lua_State* /*ls*/) {
		AssertF("no constructor defined.(can't construct this type of object)");
		return 0;
	}
}

namespace rev {
	namespace lua {
		struct Dummy {};
		template <
			class C, class Getter,
			class CBSMethod, class CBMember, class CBMethod, class Maker
		>
		void LuaExportImpl(LuaState& lsc,
				const char* class_name,
				const std::string& base_name,
				const CBSMethod& cbSMethod,
				const CBMember& cbMember,
				const CBMethod& cbMethod,
				Maker* maker
		) {
			// 既に登録されていたら警告だけ表示して何もせずリターン
			if(LuaImport::IsObjectRegistered(lsc, class_name)) {
				Expect(false, "class \"%s\" is already registered", class_name);
				return;
			}

			LuaImport::BeginImportBlock(class_name);
			lsc.getGlobal(::rev::luaNS::DefineObject);
			lsc.getGlobal(base_name);
			lsc.push(class_name);
			lsc.newTable();
			lsc.push(false);
			lsc.call(4,1);

			lsc.push(::rev::luaNS::objBase::New);
			lsc.push(maker);
			lsc.rawSet(-3);

			lsc.setField(-1, luaNS::objBase::Name, class_name);

			LuaImport::BeginImportBlock("StaticFunctions");
			cbSMethod();
			LuaImport::EndImportBlock();

			LuaImport::BeginImportBlock("Values");
			lsc.rawGetField(-1, ::rev::luaNS::objBase::ValueR);
			lsc.rawGetField(-2, ::rev::luaNS::objBase::ValueW);
			cbMember();
			lsc.pop(2);
			LuaImport::EndImportBlock();

			LuaImport::BeginImportBlock("Functions");
			lsc.rawGetField(-1, ::rev::luaNS::objBase::Func);
			cbMethod();
			lsc.pop(1);
			LuaImport::EndImportBlock();

			CallLuaExport<C>(lsc);
			lsc.setGlobal(class_name);

			LuaImport::EndImportBlock();
		}
	}
}

// param = (clazz)(class_raw)
#define DEF_LUAMEMBER(n, param, elem) LuaImport::RegisterMember<::rev::LI_GetPtr<BOOST_PP_SEQ_ELEM(0,param)>, BOOST_PP_SEQ_ELEM(0,param)>(lsc, BOOST_PP_STRINGIZE(elem), &BOOST_PP_SEQ_ELEM(0,param)::elem);
#define DEF_LUASMEMBER(n, clazz, elem) LuaImport::RegisterStatic(lsc, BOOST_PP_STRINGIZE(elem), &clazz::elem);
#define DEF_LUAIMPLEMENT_IMPL(mgr, clazz, class_raw, base, seq_smethod, seq_member, seq_method, seq_ctor, makeobj) \
namespace rev { \
	namespace lua { \
		template <> \
		const char* LuaName(clazz*) { return #class_raw; } \
		template <> \
		void LuaExport(LuaState& lsc, clazz*) { \
			LuaExportImpl<clazz, ::rev::LI_GetPtr<clazz>>( \
				lsc, \
				#class_raw, \
				base, \
				[&](){ BOOST_PP_SEQ_FOR_EACH(DEF_LUASMEMBER, clazz, seq_smethod) }, \
				[&](){ BOOST_PP_SEQ_FOR_EACH(DEF_LUAMEMBER, (clazz)(class_raw), seq_member) }, \
				[&](){ BOOST_PP_SEQ_FOR_EACH(DEF_LUAMEMBER, (clazz)(class_raw), seq_method) }, \
				&makeobj<BOOST_PP_SEQ_ENUM((mgr)(clazz) seq_ctor)> \
			); \
		} \
	} \
}

#define LUAIMPLEMENT_BASE luaNS::Object
#define DEF_LUAIMPLEMENT_SPTR(mgr, clazz, class_raw, base, seq_smethod, seq_member, seq_method, seq_ctor) \
	DEF_LUAIMPLEMENT_IMPL(mgr, clazz, class_raw, base, seq_smethod, seq_member, seq_method, seq_ctor, ::rev::MakeHandle)
#define DEF_LUAIMPLEMENT_SPOBJ(clazz, class_raw, base, seq_smethod, seq_member, seq_method, seq_ctor) \
	DEF_LUAIMPLEMENT_IMPL(::rev::lua::Dummy, clazz, class_raw, base, seq_smethod, seq_member, seq_method, seq_ctor, ::rev::MakeSPObject)
#define DEF_LUAIMPLEMENT_PTR(clazz, class_raw, base, seq_smethod, seq_member, seq_method, seq_ctor) \
	DEF_LUAIMPLEMENT_IMPL(::rev::lua::Dummy, clazz, class_raw, base, seq_smethod, seq_member, seq_method, seq_ctor, ::rev::MakeObject)
#define DEF_LUAIMPLEMENT_PTR_NOCTOR(clazz, class_raw, base, seq_smethod, seq_member, seq_method) \
	DEF_LUAIMPLEMENT_IMPL(::rev::lua::Dummy, clazz, class_raw, base, seq_smethod, seq_member, seq_method, NOTHING, ::rev::MakeFake)
