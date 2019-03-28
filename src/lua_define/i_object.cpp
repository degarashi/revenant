#include "../lua/impl.hpp"

#include "../updgroup.hpp"
#include "../object_mgr.hpp"
DEF_LUAIMPLEMENT_PTR_NOCTOR(
	rev::IObject, Object,
	LUAIMPLEMENT_BASE,
	NOTHING,
	NOTHING,
	(destroy)(getResourceName)
)
#include "../u_object.hpp"
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

#include "../object_lua.hpp"
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
#include "../drawgroup_proxy.hpp"
DEF_LUAIMPLEMENT_SPTR(
	rev::ObjMgr, rev::DrawGroupProxy, DrawGroupProxy,
	"DrawableObj",
	NOTHING,
	NOTHING,
	(setPriority)(getResourceName),
	(rev::HDGroup)
)

#include "../u_scene.hpp"
DEF_LUAIMPLEMENT_SPTR(
	rev::ObjMgr, rev::U_Scene, U_Scene,
	luaNS::FSMachine,
	NOTHING,
	NOTHING,
	(getUpdGroup)(getDrawGroup),
	NOTHING
)
#include "../scene_mgr.hpp"
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
