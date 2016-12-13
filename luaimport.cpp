#include "lcv.hpp"
#include "sdl_rw.hpp"
#include "apppath.hpp"
#include <boost/format.hpp>
#include <fstream>
extern "C" {
	#include <lauxlib.h>
}

namespace rev {
	namespace luaNS {
		const std::string ScriptResourceEntry("script"),
								SystemScriptResourceEntry("system_script"),
								ScriptExtension("lua");
		const std::string ToString("tostring");
		const std::string MakeShared("MakeShared"),
						MakeWeak("MakeWeak"),
						SetState("SetState"),
						SwitchState("SwitchState"),
						Null("Null"),
						ObjectBase("ObjectBase"),
						ConstructPtr("ConstructPtr"),
						DerivedHandle("DerivedHandle"),
						MakeFSMachine("MakeFSMachine"),
						FSMachine("FSMachine"),
						MakePreENV("MakePreENV"),
						Ctor("Ctor"),
						RecvMsg("RecvMsg"),
						RecvMsgCpp("RecvMsgCpp"),
						OnUpdate("OnUpdate"),
						OnPause("OnPause"),
						OnEffectReset("OnEffectReset"),
						OnResume("OnResume"),
						OnExit("OnExit"),
						System("System");
		namespace objBase {
			const std::string ValueR("_valueR"),
								ValueW("_valueW"),
								Func("_func"),
								UdataMT("_udata_mt"),
								MT("_mt"),
								ClassName("classname"),
								IsPointer("is_pointer"),
								Pointer("pointer"),
								Udata("udata"),
								_New("_New");
			namespace valueR {
				const std::string HandleName("handleName"),
									NumRef("numRef");
			}
		}
		namespace system {
			const std::string PathSeparation(";"),
								PathReplaceMark("?"),
								Package("package"),
								Path("path");
		}
	}

	template <class T>
	auto GetPointer(const std::shared_ptr<T>& p) {
		return p.get();
	}
	template <class T>
	auto GetPointer(const std::weak_ptr<T>& p) {
		return p.lock().get();
	}
	bool PrepareWeakTable(LuaState& lsc, const std::string& name) {
		if(lsc.prepareTableGlobal(name)) {
			const CheckTop ct(lsc.getLS());
			lsc.newTable();
			lsc.setField(-1, "__mode", "v");
			lsc.setMetatable(-2);
			return true;
		}
		return false;
	}
	template <class UD>
	bool InsertWeakTable(LuaState& lsc, const int idx, const UD& ud) {
		void* ptr = GetPointer(ud);
		lsc.getField(idx, ptr);
		const bool ret = lsc.type(-1) == LuaType::Nil;
		if(ret) {
			lsc.pop(1);
			lsc.newTable();
			lsc.push(luaNS::objBase::Udata);
			MakeUserdataWithDtor(lsc, ud);
			// [Table]["udata"][UD]
			lsc.setTable(-3);

			lsc.push(ptr);
			lsc.pushValue(-2);
			// [Table][ptr][Table]
			lsc.setTable(idx);
		}
		D_Assert0(lsc.type(-1) == LuaType::Table);
		return ret;
	}

	// [LCV<void_sp>]
	int LCV<void_sp>::operator()(lua_State* ls, const void_sp& p) const {
		if(!p) {
			// nullハンドルの場合はnilをpushする
			LCV<LuaNil>()(ls, LuaNil());
		} else {
			LuaState lsc(ls, false);
			PrepareWeakTable(lsc, "res-sp");
			InsertWeakTable(lsc, -1, p);
			// [Res][SP]
			lsc.remove(-2);
		}
		return 1;
	}
	namespace {
		template <class P>
		auto LoadSmartPtr(const int idx, lua_State* ls) {
			const RewindTop rt(ls);
			LuaState lsc(ls, false);
			const auto typ = lsc.type(idx);
			if(typ == LuaType::Nil)
				return P();
			D_Assert0(typ == LuaType::Table);
			lsc.getField(idx, luaNS::objBase::Udata);
			return *reinterpret_cast<P*>(lsc.toUserData(-1));
		}
	}
	void_sp LCV<void_sp>::operator()(const int idx, lua_State* ls, LPointerSP* /*spm*/) const {
		return LoadSmartPtr<void_sp>(idx, ls);
	}
	LuaType LCV<void_sp>::operator()(const void_sp&) const {
		return LuaType::Table;
	}
	DEF_LCV_OSTREAM(void_sp)

	// [LCV<void_wp>]
	int LCV<void_wp>::operator()(lua_State* ls, const void_wp& w) const {
		if(auto sp = w.lock()) {
			LuaState lsc(ls, false);
			PrepareWeakTable(lsc, "res-wp");
			InsertWeakTable(lsc, -1, w);
			// lockメソッドを付加
			lsc.remove(-2);
		} else {
			// nullハンドルの場合はnilをpushする
			LCV<LuaNil>()(ls, LuaNil());
		}
		return 1;
	}
	void_wp LCV<void_wp>::operator()(const int idx, lua_State* ls, LPointerSP* /*spm*/) const {
		return LoadSmartPtr<void_wp>(idx, ls);
	}
	LuaType LCV<void_wp>::operator()(const void_wp&) const {
		return LuaType::Table;
	}
	DEF_LCV_OSTREAM(void_wp)

	// ------------- LI_GetPtrBase -------------
	void* LI_GetPtrBase::operator()(lua_State* ls, const int idx) const {
		LuaState lsc(ls, true);
		lsc.getField(idx, luaNS::objBase::Pointer);
		void* ret = LCV<void*>()(-1, ls, nullptr);
		lsc.pop();
		return ret;
	}
	// ------------- LI_GetHandleBase -------------
	std::shared_ptr<void> LI_GetHandleBase::operator()(lua_State* ls, const int idx) const {
		LuaState lsc(ls, true);
		lsc.getField(idx, luaNS::objBase::Udata);
		auto ret = LCV<std::shared_ptr<void>>()(-1, ls, nullptr);
		lsc.pop();
		return ret;
	}

	// ------------- LuaImport -------------
	const char* LuaImport::HandleName(const HRes& sh) {
		return sh->getResourceName();
	}
	lua_Integer LuaImport::NumRef(const HRes& sh) {
		return static_cast<lua_Integer>(sh.use_count());
	}
	bool LuaImport::IsObjectBaseRegistered(LuaState& lsc) {
		lsc.getGlobal(luaNS::ObjectBase);
		const bool res = lsc.type(-1) == LuaType::Table;
		lsc.pop();
		return res;
	}
	int LuaImport::ReturnException(lua_State* ls, const char* func, const std::exception& e, const int nNeed) {
		return luaL_error(ls, "Error occured at\nfunction: %s\ninput argument(s): %d\nneeded argument(s): %d\n"
								"---------------- error message ----------------\n%s\n"
								"-----------------------------------------------",
								func, lua_gettop(ls), nNeed, e.what());
	}

	namespace {
		int MakeShared(lua_State*) {
			return 0;
		}
		int MakeWeak(lua_State*) {
			return 0;
		}
		int EmptyFunction(lua_State*) { return 0; }
	}
	// オブジェクト類を定義する為の基本関数定義など
	void LuaImport::RegisterObjectBase(LuaState& lsc) {
		if(IsObjectBaseRegistered(lsc))
			return;

		lsc.newTable();
		// ValueRの初期化
		// ValueR = { HandleName=(HandleName), HandleId=(HandleId), NumRef=(NumRef) }
		lsc.push(luaNS::objBase::ValueR);
		lsc.newTable();

		// HandleName
		lsc.push(luaNS::objBase::valueR::HandleName);
		LuaImport::PushFunction(lsc, &HandleName);
		lsc.setTable(-3);
		// NumRef
		lsc.push(luaNS::objBase::valueR::NumRef);
		LuaImport::PushFunction(lsc, &NumRef);
		lsc.setTable(-3);

		lsc.setTable(-3);
		// ValueWの初期化
		// ValueW = {}
		lsc.push(luaNS::objBase::ValueW);
		lsc.newTable();
		lsc.setTable(-3);
		// Funcの初期化
		// Func = {}
		lsc.push(luaNS::objBase::Func);
		lsc.newTable();
		lsc.setTable(-3);
		// RecvMsgCpp = func(RecvMsgCpp)
		lsc.push(luaNS::RecvMsgCpp);
		lsc.pushCClosure(LuaImport::RecvMsgCpp, 0);
		lsc.setTable(-3);
		// RecvMsg = func(RecvMsgCpp)
		lsc.push(luaNS::RecvMsg);
		lsc.pushCClosure(LuaImport::RecvMsgCpp, 0);
		lsc.setTable(-3);
		// Ctor = func(Ctor)
		lsc.push(luaNS::Ctor);
		lsc.pushCClosure(EmptyFunction, 0);
		lsc.setTable(-3);
		// global["ObjectBase"] = {...}
		lsc.setGlobal(luaNS::ObjectBase);

		// global["MakeShared"] = (MakeShared)
		lsc.pushCClosure(MakeShared, 0);
		lsc.setGlobal(luaNS::MakeShared);
		// global["MakeWeak"] = (MakeWeak)
		lsc.pushCClosure(MakeWeak, 0);
		lsc.setGlobal(luaNS::MakeWeak);

		lsc.loadModule("base");
	}
	bool LuaImport::IsUpdaterObjectRegistered(LuaState& lsc) {
		lsc.getGlobal(luaNS::FSMachine);
		const bool res = lsc.type(-1) == LuaType::Table;
		lsc.pop();
		return res;
	}
	void LuaImport::RegisterUpdaterObject(LuaState& lsc) {
		if(IsUpdaterObjectRegistered(lsc))
			return;
		// LuaImport::RegisterClass<Object>(lsc);

		std::string fileName("fsmachine." + luaNS::ScriptExtension);
		HRW hRW = mgr_path.getRW(luaNS::SystemScriptResourceEntry, PathBlock(fileName), Access::Read, nullptr);
		Assert(hRW, "system script file \"%s\" not found.", fileName.c_str());
		lsc.loadFromSource(hRW, fileName.c_str(), true);
	}
	void LuaImport::LoadClass(LuaState& lsc, const std::string& name, const HRW& hRW) {
		RegisterObjectBase(lsc);
		RegisterUpdaterObject(lsc);

		// グローバルテーブルの付け替え
		lsc.newTable();
		lsc.getGlobal(luaNS::MakePreENV);
		lsc.pushValue(-2);
		lsc.call(1,0);
		// ユーザーのクラス定義をスタックに積むが、まだ実行はしない
		lsc.load(hRW, (boost::format("LuaImport::LoadClass(%1%)") % name).str().c_str(), "bt", false);
		// _ENVをクラステーブルに置き換えてからチャンクを実行
		// グローバル変数に代入しようとしたらクラスのstatic変数として扱う
		lsc.pushValue(-2);
		// [NewClassTable][UserChunk][NewClassTable]
		lsc.setUpvalue(-2, 1);
		// [NewClassTable][UserChunk]
		lsc.call(0,0);
		// [NewClassTable]
		lsc.getGlobal(luaNS::MakeFSMachine);
		lsc.pushValue(-2);
		lsc.push(name);
		// [NewClassTable][Func(MakeFSMachine)][ObjName][NewClassTable]
		lsc.call(2,1);
		lsc.setGlobal(name);
		lsc.pop(1);
		return;
	}
	void LuaImport::MakePointerInstance(LuaState& lsc, const std::string& luaName, void* ptr) {
		lsc.getGlobal(luaName);
		lsc.getField(-1, luaNS::ConstructPtr);
		lsc.push(ptr);
		lsc.call(1,1);
		// [ObjDefine][Instance]
		lsc.remove(-2);
	}
	int LuaImport::RecvMsgCpp(lua_State* ls) {
		// MEMO: GameObjectを組み込んだ後にコメントアウト
		// Object* obj = LI_GetHandle<typename ObjMgr::data_type>()(ls, 1);
		// auto msg = LCV<std::string>()(2, ls);
		// // Noneで無ければ有効な戻り値とする
		// LCValue lcv = obj->recvMsg(msg, LCV<LCValue>()(3, ls));
		// if(lcv.type() != LuaType::Nil) {
		// 	lua_pushboolean(ls, true);
		// 	lcv.push(ls);
		// 	return 2;
		// }
		lua_pushboolean(ls, false);
		return 1;
	}
	LuaImport::LogMap LuaImport::s_logMap;
	std::stringstream LuaImport::s_importLog;
	std::string LuaImport::s_firstBlock;
	int LuaImport::s_indent = 0;
	void LuaImport::BeginImportBlock(const std::string& s) {
		if(s_indent > 0)
			_PushIndent(s_importLog) << '[' << s << ']' << std::endl;
		else
			s_firstBlock = s;
		++s_indent;
	}
	void LuaImport::EndImportBlock() {
		Assert(--s_indent >= 0, "indent error");
		if(s_indent == 0) {
			s_logMap[s_firstBlock].append(s_importLog.str());
			s_importLog.str("");
			s_importLog.clear();
		}
	}
	std::ostream& LuaImport::_PushIndent(std::ostream& s) {
		for(int i=0 ; i<s_indent ; i++)
			s << "    ";
		return s;
	}
	void LuaImport::SaveImportLog(const HRW& hRW) {
		std::stringstream ss;
		for(auto& e : s_logMap) {
			ss << '[' << e.first << ']' << std::endl << e.second;
		}
		auto str = ss.str();
		hRW->write(str.c_str(), 1, str.length());
		s_logMap.clear();
	}
}
