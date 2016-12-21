#pragma once
#include "lubee/meta/check_macro.hpp"
#include "lubee/meta/enable_if.hpp"
#include "lubee/error.hpp"
#include "lcv.hpp"
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
		template <class C, class Getter>
		void LuaRegisterMember(LuaState&, const std::tuple<>&) {}
		template <class C, class Getter, class Tag, class... Remain>
		void LuaRegisterMember(LuaState& lsc, const std::tuple<Tag, Remain...>&) {
			LuaImport::RegisterMember<Getter, C>(lsc, Tag::name, Tag::pointer);
			LuaRegisterMember<C, Getter>(lsc, std::tuple<Remain...>());
		}

		template <
			class C, class Getter,
			class...Member, class... Method, class Maker
		>
		void LuaExportImpl(LuaState& lsc,
				const char* class_name,
				const char* base_name,
				const std::tuple<Member...>& member,
				const std::tuple<Method...>& method,
				Maker* maker
		) {
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

			LuaImport::BeginImportBlock("Values");
			lsc.rawGetField(-1, ::rev::luaNS::objBase::ValueR);
			lsc.rawGetField(-2, ::rev::luaNS::objBase::ValueW);
			LuaRegisterMember<C, Getter>(lsc, member);
			lsc.pop(2);
			LuaImport::EndImportBlock();

			LuaImport::BeginImportBlock("Functions");
			lsc.rawGetField(-1, ::rev::luaNS::objBase::Func);
			LuaRegisterMember<C, Getter>(lsc, method);
			lsc.pop(1);
			LuaImport::EndImportBlock();

			CallLuaExport<C>(lsc);
			lsc.setGlobal(class_name);

			LuaImport::EndImportBlock();
		}
	}
}

// param = (class_raw)(clazz)
#define DEF_MEMBER_NAME(n, param, elem)	\
	struct BOOST_PP_CAT(BOOST_PP_CAT(BOOST_PP_SEQ_ELEM(0, param), elem), _tag) { \
		static const char* name; \
		static decltype(&BOOST_PP_SEQ_ELEM(1, param)::elem) pointer; \
	}; \
	const char* BOOST_PP_CAT(BOOST_PP_CAT(BOOST_PP_SEQ_ELEM(0, param), elem), _tag)::name = #elem; \
	decltype(BOOST_PP_CAT(BOOST_PP_CAT(BOOST_PP_SEQ_ELEM(0, param), elem), _tag)::pointer) \
		BOOST_PP_CAT(BOOST_PP_CAT(BOOST_PP_SEQ_ELEM(0, param), elem), _tag)::pointer = \
		&BOOST_PP_SEQ_ELEM(1, param)::elem;

#define SEQUENCE(S) BOOST_PP_IIF(BOOST_PP_EQUAL(0, BOOST_PP_SEQ_SIZE(S)), (), S)
#define ENUM_NAME(n, class_raw, elem) (BOOST_PP_CAT(BOOST_PP_CAT(class_raw, elem), _tag))
#define DEF_LUAIMPLEMENT_IMPL(mgr, clazz, class_raw, base, seq_member, seq_method, seq_ctor, makeobj) \
namespace rev { \
	namespace lua { \
		template <> \
		const char* LuaName(clazz*) { return #class_raw; } \
		BOOST_PP_SEQ_FOR_EACH(DEF_MEMBER_NAME, (class_raw)(clazz), seq_member) \
		BOOST_PP_SEQ_FOR_EACH(DEF_MEMBER_NAME, (class_raw)(clazz), seq_method) \
		template <> \
		void LuaExport(LuaState& lsc, clazz*) { \
			LuaExportImpl<clazz, ::rev::LI_GetPtr<clazz>>( \
				lsc, \
				#class_raw, \
				#base, \
				std::tuple<BOOST_PP_SEQ_ENUM(SEQUENCE(BOOST_PP_SEQ_FOR_EACH(ENUM_NAME, class_raw, seq_member)))>(), \
				std::tuple<BOOST_PP_SEQ_ENUM(SEQUENCE(BOOST_PP_SEQ_FOR_EACH(ENUM_NAME, class_raw, seq_method)))>(), \
				&makeobj<BOOST_PP_SEQ_ENUM((mgr)(clazz) seq_ctor)> \
			); \
		} \
	} \
}

#define LUAIMPLEMENT_BASE Object
#define DEF_LUAIMPLEMENT_SPTR(mgr, clazz, class_raw, base, seq_member, seq_method, seq_ctor) \
	DEF_LUAIMPLEMENT_IMPL(mgr, clazz, class_raw, base, seq_member, seq_method, seq_ctor, ::rev::MakeHandle)
#define DEF_LUAIMPLEMENT_PTR(clazz, class_raw, base, seq_member, seq_method, seq_ctor) \
	DEF_LUAIMPLEMENT_IMPL(::rev::lua::Dummy, clazz, class_raw, base, seq_member, seq_method, seq_ctor, ::rev::MakeObject)
#define DEF_LUAIMPLEMENT_PTR_NOCTOR(clazz, class_raw, base, seq_member, seq_method) \
	DEF_LUAIMPLEMENT_IMPL(::rev::lua::Dummy, clazz, class_raw, base, seq_member, seq_method, NOTHING, ::rev::MakeFake)
