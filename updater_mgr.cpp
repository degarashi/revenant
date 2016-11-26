#include "updater_mgr.hpp"
#include "updater.hpp"

namespace rev {
	ObjMgr::~ObjMgr() {
		constexpr int N_RETRY = 8;
		for(int i=0 ; i<N_RETRY ; i++) {
			// オブジェクトが無くなるまで繰り返しdestroy
			auto tmp = base_t::getResourceSet();
			if(tmp->set.empty())
				break;
			for(auto& obj : tmp->set) {
				try {
					if(auto sp = obj.weak.lock())
						sp->destroy();
				} catch(...) {}
			}
		}
	}
	void ObjMgr::setLua(const Lua_SP& ls) {
		_lua = ls;
	}
	const Lua_SP& ObjMgr::getLua() const noexcept {
		return _lua;
	}
}
#include "updater_lua.hpp"
namespace rev {
	// -------------------- ObjectT_LuaBase --------------------
	LCValue detail::ObjectT_LuaBase::CallRecvMsg(const Lua_SP& ls, const HObj& hObj, const GMessageStr& msg, const LCValue& arg) {
		ls->push(hObj);
		LValueS lv(ls->getLS());
		LCValue ret = lv.callMethodNRet(luaNS::RecvMsg, msg, arg);
		auto& tbl = *boost::get<LCTable_SP>(ret);
		if(tbl[1]) {
			const int sz = tbl.size();
			for(int i=1 ; i<=sz ; i++)
				tbl[i] = std::move(tbl[i+1]);
			tbl.erase(tbl.find(sz));
			return ret;
		}
		return LCValue();
	}
	// -------------------- U_ObjectUpd --------------------
	struct U_ObjectUpd::St_None : StateT<St_None> {};
	U_ObjectUpd::U_ObjectUpd() {
		setStateNew<St_None>();
	}
}
