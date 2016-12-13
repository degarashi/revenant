#pragma once
#include "lubee/meta/check_macro.hpp"
#include "lubee/meta/enable_if.hpp"
#include "lubee/error.hpp"
#include <boost/preprocessor.hpp>
#include "lcv.hpp"
#include <lua.hpp>

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
#define DEF_LUAIMPLEMENT_DERIVED(clazz, der) \
	namespace rev { \
		namespace lua { \
			template <> \
			const char* LuaName(clazz*) { return LuaName((der*)nullptr); } \
			template <> \
			void LuaExport(LuaState& lsc, clazz*) { LuaExport(lsc, (der*)nullptr); } \
		} \
	}

namespace rev {
	template <class T, class... Ts>
	int MakeObject(lua_State* ls) {
		T ret = FuncCall<Ts...>::callCB(
			[](auto&&... args){
				return T{std::forward<decltype(args)>(args)...};
			},
			ls,
			static_cast<int>(-sizeof...(Ts))
		);
		LCV<T>()(ls, std::move(ret));
		return 1;
	}
	//! (LuaのClassT::New()から呼ばれる)オブジェクトのリソースハンドルを作成
	template <class Mgr_t, class TF, class... Ts>
	int MakeHandle(lua_State* ls) {
		auto hObj = FuncCall<Ts...>::callCB(
			[](auto&&... args) {
				return Mgr_t::ref().template emplace<TF>(std::forward<decltype(args)>(args)...);
			},
			ls,
			static_cast<int>(-sizeof...(Ts))
		);
		LCV<HRes>()(ls, hObj);
		return 1;
	}
	//! デフォルトコンストラクタを持たないオブジェクト用
	template <class... Ts>
	int MakeHandle_Fake(lua_State* /*ls*/) {
		AssertF("no constructor defined.(can't construct this type of object)");
		return 0;
	}
}

#define DEF_REGMEMBER(n, clazz, elem, getter)	::rev::LuaImport::RegisterMember<getter,clazz>(lsc, BOOST_PP_STRINGIZE(elem), &clazz::elem);
#define DEF_REGMEMBER_HDL(n, data, elem)	DEF_REGMEMBER(n, BOOST_PP_SEQ_ELEM(1,data), elem, ::rev::LI_GetHandle<BOOST_PP_SEQ_ELEM(0,data)>)
#define DEF_REGMEMBER_PTR(n, clazz, elem)	DEF_REGMEMBER(n, clazz, elem, ::rev::LI_GetPtr<clazz>)

#define DEF_LUAIMPLEMENT_HDL_IMPL(mgr, clazz, class_name, base, seq_member, seq_method, seq_ctor, makeobj) \
	namespace rev { \
		namespace lua { \
			template <> \
			const char* LuaName(clazz*) { return #class_name; } \
			template <> \
			void LuaExport(LuaState& lsc, clazz*) { \
				LuaImport::BeginImportBlock(#class_name); \
				lsc.getGlobal(::rev::luaNS::DerivedHandle); \
				lsc.getGlobal(base); \
				lsc.push(#class_name); \
				lsc.prepareTableGlobal(#class_name); \
				lsc.call(3,1); \
				lsc.push(::rev::luaNS::objBase::_New); \
				lsc.push(makeobj<BOOST_PP_SEQ_ENUM((mgr)(clazz)seq_ctor)>); \
				lsc.rawSet(-3); \
				lsc.setField(-1, luaNS::objBase::IsPointer, false); \
				lsc.setField(-1, luaNS::objBase::ClassName, #class_name); \
				\
				LuaImport::BeginImportBlock("Values"); \
				lsc.rawGetField(-1, ::rev::luaNS::objBase::ValueR); \
				lsc.rawGetField(-2, ::rev::luaNS::objBase::ValueW); \
				BOOST_PP_SEQ_FOR_EACH(DEF_REGMEMBER_HDL, (typename mgr::data_type)(clazz), seq_member) \
				lsc.pop(2); \
				LuaImport::EndImportBlock(); \
				\
				LuaImport::BeginImportBlock("Functions"); \
				lsc.rawGetField(-1, ::rev::luaNS::objBase::Func); \
				BOOST_PP_SEQ_FOR_EACH(DEF_REGMEMBER_HDL, (typename mgr::data_type)(clazz), seq_method) \
				lsc.pop(1); \
				LuaImport::EndImportBlock(); \
				\
				CallLuaExport<clazz>(lsc); \
				lsc.pop(1); \
				LuaImport::EndImportBlock(); \
			} \
		} \
	}

#define DEF_LUAIMPLEMENT_HDL(mgr, clazz, class_name, base, seq_member, seq_method, seq_ctor) \
	DEF_LUAIMPLEMENT_HDL_IMPL(mgr, clazz, class_name, base, seq_member, seq_method, seq_ctor, ::rev::MakeHandle)
#define DEF_LUAIMPLEMENT_HDL_NOBASE(mgr, clazz, class_name, seq_member, seq_method, seq_ctor) \
	DEF_LUAIMPLEMENT_HDL(mgr, clazz, class_name, ::rev::luaNS::ObjectBase, seq_member, seq_method, seq_ctor)
#define DEF_LUAIMPLEMENT_HDL_NOCTOR(mgr, clazz, class_name, base, seq_member, seq_method) \
	DEF_LUAIMPLEMENT_HDL_IMPL(mgr, clazz, class_name, base, seq_member, seq_method, NOTHING, ::rev::MakeHandle_Fake)
#define DEF_LUAIMPLEMENT_HDL_NOBASE_NOCTOR(mgr, clazz, class_name, seq_member, seq_method) \
	DEF_LUAIMPLEMENT_HDL_IMPL(mgr, clazz, class_name, ::rev::luaNS::ObjectBase, seq_member, seq_method, NOTHING, ::rev::MakeHandle_Fake)

#define DEF_LUAIMPLEMENT_PTR_NOCTOR(clazz, class_name, seq_member, seq_method) \
	DEF_LUAIMPLEMENT_PTR_BASE(clazz, class_name, seq_member, seq_method, MakeHandle_Fake, NOTHING)
#define DEF_LUAIMPLEMENT_PTR(clazz, class_name, seq_member, seq_method, seq_ctor) \
	DEF_LUAIMPLEMENT_PTR_BASE(clazz, class_name, seq_member, seq_method, MakeObject, seq_ctor)
#define DEF_LUAIMPLEMENT_PTR_BASE(clazz, class_name, seq_member, seq_method, makeobj, seq_ctor) \
	namespace rev { \
		namespace lua { \
			template <> \
			const char* LuaName(clazz*) { return #class_name; } \
			template <> \
			void LuaExport(LuaState& lsc, clazz*) { \
				LuaImport::BeginImportBlock(#class_name); \
				lsc.getGlobal(::rev::luaNS::DerivedHandle); \
				lsc.getGlobal(::rev::luaNS::ObjectBase); \
				lsc.push(#class_name); \
				lsc.prepareTableGlobal(#class_name); \
				lsc.call(3,1); \
				\
				lsc.push(::rev::luaNS::objBase::_New); \
				lsc.push(makeobj<BOOST_PP_SEQ_ENUM((clazz)seq_ctor)>); \
				lsc.rawSet(-3); \
				lsc.setField(-1, luaNS::objBase::IsPointer, true); \
				lsc.setField(-1, luaNS::objBase::ClassName, #class_name); \
				\
				LuaImport::BeginImportBlock("Values"); \
				lsc.rawGetField(-1, ::rev::luaNS::objBase::ValueR); \
				lsc.rawGetField(-2, ::rev::luaNS::objBase::ValueW); \
				BOOST_PP_SEQ_FOR_EACH(DEF_REGMEMBER_PTR, clazz, seq_member) \
				lsc.pop(2); \
				LuaImport::EndImportBlock(); \
				\
				LuaImport::BeginImportBlock("Functions"); \
				lsc.rawGetField(-1, ::rev::luaNS::objBase::Func); \
				BOOST_PP_SEQ_FOR_EACH(DEF_REGMEMBER_PTR, clazz, seq_method) \
				lsc.pop(1); \
				LuaImport::EndImportBlock(); \
				\
				CallLuaExport<clazz>(lsc); \
				lsc.pop(1); \
				LuaImport::EndImportBlock(); \
			} \
		} \
	}
