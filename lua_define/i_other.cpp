#include "../luaimpl.hpp"

#include "../lsys.hpp"
DEF_LUAIMPLEMENT_PTR_NOCTOR(
	rev::LSysFunc, LSysFunc,
	LUAIMPLEMENT_BASE,
	NOTHING,
	NOTHING,
	(loadResource)(loadResources)(loadResourcesASync)(queryProgress)(getResult)(getNTask)(sleep)(loadClass)
)

#include "../pose2d.hpp"
DEF_LUAIMPLEMENT_PTR_NOCTOR(
	rev::Pose2D, Pose2D,
	LUAIMPLEMENT_BASE,
	NOTHING,
	NOTHING,
	(getOffset)(getScaling)(getRotation)(getUp)(getRight)(getToWorld)(getToLocal)
	(refScaling)(refRotation)(refOffset)
	(setScaling<const frea::AVec2&>)(setRotation<const frea::RadF&>)(setOffset<const frea::AVec2&>)(setUp)
	(identity)(moveUp)(moveDown)(moveLeft)(moveRight)(lerp)(equal)(toString)
)

#include "../pose3d.hpp"
DEF_LUAIMPLEMENT_PTR_NOCTOR(
	rev::Pose3D, Pose3D,
	LUAIMPLEMENT_BASE,
	NOTHING,
	NOTHING,
	(getOffset)(getRotation)(getScaling)(getToWorld)(getToLocal)(getUp)(getRight)(getDir)
	(refOffset)(refRotation)(refScaling)
	(setAll)(setScaling<const frea::AVec3&>)(setRotation<const frea::AQuat&>)(addAxisRotation)(setOffset<const frea::AVec3&>)(addOffset)
	(identity)(moveFwd2D)(moveSide2D)(moveFwd3D)(moveSide3D)(turnAxis)(turnYPR)(addRotation)(lerpTurn)(adjustNoRoll)(lerp)(equal)(toString)
)

#include "../camera3d.hpp"
DEF_LUAIMPLEMENT_SPTR(
	rev::Camera3DMgr, rev::Camera3D, Camera3D,
	LUAIMPLEMENT_BASE,
	NOTHING,
	NOTHING,
	(setPose<const Pose3D&>)(setFov<const frea::RadF&>)(setAspect<const float&>)(setNearZ<const float&>)(setFarZ<const float&>)(setZPlane)
	(refPose)
	(getPose)(getFov)(getAspect)(getNearZ)(getFarZ)
	(unproject)(unprojectVec)(vp2wp),
	NOTHING
)

#include "../camera2d.hpp"
DEF_LUAIMPLEMENT_SPTR(
	rev::Camera2DMgr, rev::Camera2D, Camera2D,
	LUAIMPLEMENT_BASE,
	NOTHING,
	NOTHING,
	(setPose<const Pose2D&>)(setAspectRatio<const float&>)
	(refPose)
	(getPose)(getAspectRatio)
	(vp2w)(v2w),
	NOTHING
)

#include "../drawtoken/clear.hpp"
DEF_LUAIMPLEMENT_PTR(
	rev::draw::ClearParam, ClearParam,
	LUAIMPLEMENT_BASE,
	NOTHING,
	(color)(depth)(stencil),
	NOTHING,
	(spi::Optional<frea::Vec4>)(spi::Optional<float>)(spi::Optional<uint32_t>)
)

#include "../systeminfo.hpp"
DEF_LUAIMPLEMENT_PTR_NOCTOR(
	rev::SystemInfo, SystemInfo,
	LUAIMPLEMENT_BASE,
	NOTHING,
	NOTHING,
	(getScreenSize)(getFPS)
)

namespace rev {
	void LuaImport::RegisterOtherClass(LuaState& lsc) {
		RegisterClass<Pose2D>(lsc);
		RegisterClass<Pose3D>(lsc);
		RegisterClass<Camera2D>(lsc);
		RegisterClass<Camera3D>(lsc);
		RegisterClass<draw::ClearParam>(lsc);
		ImportClass(lsc, "System", "lsys", &mgr_lsys);
		ImportClass(lsc, "System", "info", &mgr_info);
	}
}