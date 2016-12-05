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
								_Pointer("_pointer"),
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

	// [LCV<HRes>]
	int LCV<HRes>::operator()(lua_State* ls, const HRes& s) const {
		if(!s) {
			// nullハンドルの場合はnilをpushする
			LCV<LuaNil>()(ls, LuaNil());
		} else {
			const auto& name = s->getResourceName();
			D_Assert(name, "invaild resource name.");
			LuaState lsc(ls, true);
			lsc.getGlobal(name);
			lsc.getField(-1, luaNS::MakeShared);
			new(lsc.newUserData(sizeof(HRes))) HRes(s);
			lsc.call(1,1);
			lsc.remove(-2);
			D_Assert0(lsc.type(-1) == LuaType::Userdata);
		}
		return 1;
	}
	HRes LCV<HRes>::operator()(const int idx, lua_State* ls, LPointerSP* spm) const {
		const auto typ = lua_type(ls, idx);
		if(typ != LUA_TNIL) {
			if(auto* res = static_cast<HRes*>(LCV<void*>()(idx, ls, spm)))
				return *res;
		}
		return HRes();
	}
	LuaType LCV<HRes>::operator()() const {
		return LuaType::Userdata;
	}
	DEF_LCV_OSTREAM(HRes)

	// [LCV<WRes>]
	int LCV<WRes>::operator()(lua_State* ls, const WRes& w) const {
		if(auto sp = w.lock()) {
			LuaState lsc(ls, true);
			const auto& name = sp->getResourceName();
			D_Assert(name, "invaild resource name.");
			lsc.getGlobal(name);
			lsc.getField(-1, luaNS::MakeWeak);
			new(lsc.newUserData(sizeof(WRes))) WRes(w);
			lsc.call(1,1);
			lsc.remove(-2);
			D_Assert0(lsc.type(-1) == LuaType::Userdata);
		} else {
			// nullハンドルの場合はnilをpushする
			LCV<LuaNil>()(ls, LuaNil());
		}
		return 1;
	}
	WRes LCV<WRes>::operator()(const int idx, lua_State* ls, LPointerSP* spm) const {
		const auto typ = lua_type(ls, idx);
		if(typ != LUA_TNIL) {
			if(auto* res = static_cast<WRes*>(LCV<void*>()(idx, ls, spm)))
				return *res;
		}
		return WRes();
	}
	LuaType LCV<WRes>::operator()() const {
		return LuaType::LightUserdata;
	}
	DEF_LCV_OSTREAM(WRes)

	namespace {
		#ifdef DEBUG
			void CheckClassName(lua_State* ls, const int idx, const char* name) {
				LuaState lsc(ls, true);
				lsc.getField(idx, luaNS::objBase::ClassName);
				auto objname = lsc.toString(-1);
				Assert(objname==name, "invalid object (required %s, but got %s)", name, objname.c_str());
				lsc.pop();
			}
		#else
			void CheckClassName(lua_State*, int, const char*) {}
		#endif
	}
	// ------------- LI_GetPtrBase -------------
	void* LI_GetPtrBase::operator()(lua_State* ls, const int idx, const char* name) const {
		CheckClassName(ls, idx, name);
		return LCV<void*>()(-1, ls);
	}
	// ------------- LI_GetHandleBase -------------
	HRes LI_GetHandleBase::operator()(lua_State* ls, const int idx, const char* name) const {
		CheckClassName(ls, idx, name);
		return LCV<HRes>()(-1, ls);
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
