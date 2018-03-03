#include "../luaimpl.hpp"

#include "../updgroup.hpp"
#include "../updater_mgr.hpp"
DEF_LUAIMPLEMENT_PTR_NOCTOR(
	rev::Object, Object,
	LUAIMPLEMENT_BASE,
	NOTHING,
	NOTHING,
	(destroy)(getResourceName)
)
DEF_LUAIMPLEMENT_SPTR(
	rev::ObjMgr, rev::U_Object, U_Object,
	luaNS::FSMachine,
	NOTHING,
	NOTHING,
	NOTHING,
	NOTHING
)
DEF_LUAIMPLEMENT_PTR_NOCTOR(
	rev::UpdGroup, UpdGroup,
	LUAIMPLEMENT_BASE,
	NOTHING,
	NOTHING,
	(addObj)(addObjPriority)(remObj)
	(getResourceName)(getPriority)(clear)(getNMember)
)
DEF_LUAIMPLEMENT_SPTR(
	rev::ObjMgr, rev::U_UpdGroup, U_UpdGroup,
	"UpdGroup",
	NOTHING,
	NOTHING,
	NOTHING,
	NOTHING
)

#include "../updater_lua.hpp"
DEF_LUAIMPLEMENT_SPTR(
	rev::ObjMgr, rev::U_ObjectUpd, U_ObjectUpd,
	luaNS::FSMachine,
	NOTHING,
	NOTHING,
	NOTHING,
	NOTHING
)

#include "../drawable.hpp"
DEF_LUAIMPLEMENT_PTR_NOCTOR(
	rev::DrawableObj, DrawableObj,
	LUAIMPLEMENT_BASE,
	NOTHING,
	NOTHING,
	(setDrawPriority)
)
#include "../drawgroup.hpp"
DEF_LUAIMPLEMENT_PTR_NOCTOR(
	rev::DrawGroup, DrawGroup,
	"DrawableObj",
	NOTHING,
	NOTHING,
	(addObj)(remObj)(setSortAlgorithmId)
	(getResourceName)(getNMember)
	(clear)
)
#include "../drawgroup_proxy.hpp"
DEF_LUAIMPLEMENT_PTR_NOCTOR(
	rev::DrawGroupProxy, DrawGroupProxy,
	"DrawableObj",
	NOTHING,
	NOTHING,
	(setPriority)(getResourceName)
)
DEF_LUAIMPLEMENT_SPTR(
	rev::ObjMgr, rev::U_DrawGroup, U_DrawGroup,
	"DrawGroup",
	NOTHING,
	NOTHING,
	NOTHING,
	(const SortAlgList&)(bool)
)
DEF_LUAIMPLEMENT_SPTR(
	rev::ObjMgr, rev::U_DrawGroupProxy, U_DrawGroupProxy,
	"DrawGroupProxy",
	NOTHING,
	NOTHING,
	NOTHING,
	(rev::HDGroup)
)

#include "../scene.hpp"
DEF_LUAIMPLEMENT_SPTR(
	rev::ObjMgr, rev::U_Scene, U_Scene,
	luaNS::FSMachine,
	NOTHING,
	NOTHING,
	(getUpdGroup)(getDrawGroup),
	NOTHING
)
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
		RegisterClass<U_UpdGroup>(lsc);
		RegisterClass<U_ObjectUpd>(lsc);
		RegisterClass<DrawableObj>(lsc);
		RegisterClass<DrawGroup>(lsc);
		RegisterClass<DrawGroupProxy>(lsc);
		RegisterClass<U_DrawGroup>(lsc);
		RegisterClass<U_DrawGroupProxy>(lsc);
		RegisterClass<U_Scene>(lsc);
		ImportClass(lsc, "System", "scene", &mgr_scene);
	}
}
