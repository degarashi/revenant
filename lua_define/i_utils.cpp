#include "../luaimpl.hpp"
#include "../object_mgr.hpp"

#include "../util/fbclear.hpp"
DEF_LUAIMPLEMENT_SPTR(
	rev::ObjMgr, rev::util::FBClear, FBClear,
	LUAIMPLEMENT_BASE,
	NOTHING,
	NOTHING,
	NOTHING,
	(rev::Priority)(const rev::draw::ClearParam&)
)

#include "../util/fbswitch.hpp"
DEF_LUAIMPLEMENT_SPTR(
	rev::ObjMgr, rev::util::FBSwitch, FBSwitch,
	LUAIMPLEMENT_BASE,
	NOTHING,
	NOTHING,
	(setClearParam),
	(rev::Priority)(const HFb&)(const rev::util::ClearParam_OP&)
)

namespace rev {
	void LuaImport::RegisterUtilsClass(LuaState& lsc) {
		RegisterClass<util::FBSwitch>(lsc);
		RegisterClass<util::FBClear>(lsc);
	}
}
