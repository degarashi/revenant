#include "../lua/impl.hpp"
#include "../object/mgr.hpp"

#include "../drawutil/fbclear.hpp"
DEF_LUAIMPLEMENT_SPTR(
	rev::ObjMgr, rev::drawutil::FBClear, FBClear,
	LUAIMPLEMENT_BASE,
	NOTHING,
	NOTHING,
	NOTHING,
	(rev::Priority)(const rev::ClearParam&)
)

#include "../drawutil/fbswitch.hpp"
DEF_LUAIMPLEMENT_SPTR(
	rev::ObjMgr, rev::drawutil::FBSwitch, FBSwitch,
	LUAIMPLEMENT_BASE,
	NOTHING,
	NOTHING,
	(setClearParam),
	(rev::Priority)(const HFb&)(const rev::drawutil::ClearParam_OP&)
)

namespace rev {
	void LuaImport::RegisterUtilsClass(LuaState& lsc) {
		RegisterClass<drawutil::FBSwitch>(lsc);
		RegisterClass<drawutil::FBClear>(lsc);
	}
}
