#include "../lua/impl.hpp"

#include "../object/updgroup.hpp"
#include "../object/mgr.hpp"
DEF_LUAIMPLEMENT_PTR_NOCTOR(
	rev::IObject, Object,
	LUAIMPLEMENT_BASE,
	NOTHING,
	NOTHING,
	(destroy)(getResourceName)
)
#include "../object/user.hpp"
DEF_LUAIMPLEMENT_SPTR(
	rev::ObjMgr, rev::U_Object, U_Object,
	luaNS::FSMachine,
	NOTHING,
	NOTHING,
	NOTHING,
	NOTHING
)
DEF_LUAIMPLEMENT_SPTR(
	rev::ObjMgr, rev::UpdGroup, UpdGroup,
	LUAIMPLEMENT_BASE,
	NOTHING,
	NOTHING,
	(addObj)(addObjPriority)(remObj)
	(getResourceName)(getPriority)(clear)(getNMember),
	NOTHING
)

#include "../object/lua.hpp"
DEF_LUAIMPLEMENT_SPTR(
	rev::ObjMgr, rev::U_ObjectUpd, U_ObjectUpd,
	luaNS::FSMachine,
	NOTHING,
	NOTHING,
	NOTHING,
	NOTHING
)

#include "../object/drawable.hpp"
DEF_LUAIMPLEMENT_PTR_NOCTOR(
	rev::DrawableObj, DrawableObj,
	LUAIMPLEMENT_BASE,
	NOTHING,
	NOTHING,
	(setDrawPriority)
)
#include "../object/drawgroup.hpp"
DEF_LUAIMPLEMENT_SPTR(
	rev::ObjMgr, rev::DrawGroup, DrawGroup,
	"DrawableObj",
	NOTHING,
	NOTHING,
	(addObj)(remObj)(setSortAlgorithmId)
	(getResourceName)(getNMember)
	(clear),
	(const SortAlgList&)(bool)
)
#include "../object/drawgroup_proxy.hpp"
DEF_LUAIMPLEMENT_SPTR(
	rev::ObjMgr, rev::DrawGroupProxy, DrawGroupProxy,
	"DrawableObj",
	NOTHING,
	NOTHING,
	(setPriority)(getResourceName),
	(rev::HDGroup)
)

#include "../object/scene/user.hpp"
DEF_LUAIMPLEMENT_SPTR(
	rev::ObjMgr, rev::U_Scene, U_Scene,
	luaNS::FSMachine,
	NOTHING,
	NOTHING,
	(getUpdGroup)(getDrawGroup),
	NOTHING
)
#include "../object/scene/mgr.hpp"
DEF_LUAIMPLEMENT_PTR_NOCTOR(
	rev::SceneMgr, SceneMgr,
	LUAIMPLEMENT_BASE,
	NOTHING,
	NOTHING,
	(isEmpty)(getTop)(getScene)(setPushScene)(setPopScene)
)

namespace rev {
	void LuaImport::RegisterUpdaterClass(LuaState& lsc) {
		RegisterFSMachineBase(lsc);
		RegisterClass<U_Object>(lsc);
		RegisterClass<UpdGroup>(lsc);
		RegisterClass<U_ObjectUpd>(lsc);
		RegisterClass<DrawableObj>(lsc);
		RegisterClass<DrawGroup>(lsc);
		RegisterClass<DrawGroupProxy>(lsc);
		RegisterClass<U_Scene>(lsc);
		ImportClass(lsc, "System", "scene", &mgr_scene);
	}
}
