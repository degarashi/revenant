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
		const std::string HasSP("HasSP"),
						MakeSP("MakeSP"),
						GetSP("GetSP"),
						HasWP("HasWP"),
						MakeWP("MakeWP"),
						GetWP("GetWP"),
						Void("Void");
		const std::string ScriptResourceEntry("script"),
								SystemScriptResourceEntry("system_script"),
								ScriptExtension("lua");
		const std::string ToString("tostring");
		const std::string SetState("SetState"),
						SwitchState("SwitchState"),
						Null("Null"),
						Object("Object"),
						ConstructPtr("ConstructPtr"),
						DefineObject("DefineObject"),
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
						System("System"),
						Global("Global"),
						Postfix("_postfix");
		namespace objBase {
			const std::string ValueR("_valueR"),
								ValueW("_valueW"),
								Func("_func"),
								Metatable("_metatable"),
								Base("_base"),
								Name("_name"),
								Pointer("_pointer"),
								Udata("_udata"),
								New("_new");
			namespace valueR {
				const std::string NumRef("numRef");
			}
		}
		namespace postfix {
			const std::string Vector("V"),
								Matrix("M"),
								Quat("Q"),
								ExpQuat("E"),
								Plane("P"),
								Degree("D"),
								Radian("R");
		}
		namespace vector {
			const std::string Size("_size");
		}
		namespace system {
			const std::string PathSeparation(";"),
								PathReplaceMark("?"),
								Package("package"),
								Path("path");
		}
	}
	// ------------- LuaImport -------------
	lua_Integer LuaImport::NumRef(const HRes& sh) {
		return static_cast<lua_Integer>(sh.use_count());
	}
	bool LuaImport::_IsObjectRegistered(LuaState& lsc, const std::string& name) {
		lsc.getGlobal(name);
		const bool res = lsc.type(-1) == LuaType::Table;
		lsc.pop();
		return res;
	}

	int LuaImport::_ReturnException(lua_State* ls, const char* func, const std::exception& e, const int nNeed) {
		return luaL_error(ls, "Error occured at\nfunction: %s\ninput argument(s): %d\nneeded argument(s): %d\n"
								"---------------- error message ----------------\n%s\n"
								"-----------------------------------------------",
								func, lua_gettop(ls), nNeed, e.what());
	}
	namespace {
		int EmptyFunction(lua_State*) {
			return 0;
		}
	}

	// オブジェクト類を定義する為の基本関数定義など
	void LuaImport::RegisterObjectBase(LuaState& lsc) {
		if(_IsObjectRegistered(lsc, luaNS::Object))
			return;

		lsc.newTable();
		// {_name = "Object"}
		{
			lsc.push(luaNS::objBase::Name);
			lsc.push(luaNS::Object);
			lsc.setTable(-3);
		}
		// ValueRの初期化
		// ValueR = { NumRef=(NumRef) }
		{
			lsc.push(luaNS::objBase::ValueR);
			lsc.newTable();

			// NumRef
			lsc.push(luaNS::objBase::valueR::NumRef);
			LuaImport::PushFunction(lsc, &NumRef);
			lsc.setTable(-3);

			lsc.setTable(-3);
		}
		// ValueWの初期化
		// ValueW = {}
		{
			lsc.push(luaNS::objBase::ValueW);
			lsc.newTable();
			lsc.setTable(-3);
		}
		// Funcの初期化
		// Func = {}
		{
			lsc.push(luaNS::objBase::Func);
			lsc.newTable();
			lsc.setTable(-3);
		}
		// RecvMsgCpp = func(RecvMsgCpp)
		{
			lsc.push(luaNS::RecvMsgCpp);
			lsc.pushCClosure(LuaImport::RecvMsgCpp, 0);
			lsc.setTable(-3);
		}
		// RecvMsg = func(RecvMsgCpp)
		{
			lsc.push(luaNS::RecvMsg);
			lsc.pushCClosure(LuaImport::RecvMsgCpp, 0);
			lsc.setTable(-3);
		}
		{
			// Ctor = func(Ctor)
			lsc.push(luaNS::Ctor);
			lsc.push(&EmptyFunction);
			lsc.setTable(-3);
		}
		// global["Object"] = {...}
		lsc.setGlobal(luaNS::Object);

		lsc.loadModule("object");
	}
	void LuaImport::RegisterUpdaterObject(LuaState& lsc) {
		if(_IsObjectRegistered(lsc, luaNS::FSMachine))
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
	void LuaImport::MakePointerInstance(LuaState& lsc, const char* luaName, void* ptr) {
		lsc.getGlobal(luaName);
		lsc.getField(-1, luaNS::ConstructPtr);
		lsc.push(ptr);
		// [ObjectTable][ConstructPtr][UData]
		lsc.call(1,1);
		// [ObjectTable][Instance]
		lsc.remove(-2);
	}
	void LuaImport::MakeInstance(LuaState& lsc, const char* luaName, int idx) {
		idx = lsc.absIndex(idx);
		lsc.getGlobal(luaName);
		lsc.getField(-1, luaNS::ConstructPtr);
		lsc.pushValue(idx);
		// [ObjectTable][ConstructPtr][UData]
		lsc.call(1,1);
		// [ObjectTable][Instance]
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
