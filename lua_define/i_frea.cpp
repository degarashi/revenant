#include "../luaimpl.hpp"
#include "frea/vector.hpp"

DEF_LUAIMPLEMENT_PTR(
	frea::Vec2, Vec2,
	LUAIMPLEMENT_BASE,
	(Ccw)(Cw),
	(x)(y),
	(dot)(average)(distance)(dist_sq)(getMin)(selectMin)(getMax)(selectMax)(normalize)(normalization)
	(length)(len_sq)(isNaN)(isOutstanding)(saturation)(l_intp)(absolute)(getMinValue)(getMaxValue)
	(linearNormalize)(linearNormalization)(isZero)
	(luaAddV)(luaSubV)(luaMulF)(luaMulV)(luaMulM)(luaDivF)(luaDivV)(luaInvert)(luaEqual)(luaToString)
	(convert<3>)(convert<4>),
	(float)(float)
)
DEF_LUAIMPLEMENT_PTR(
	frea::Vec3, Vec3,
	LUAIMPLEMENT_BASE,
	NOTHING,
	(x)(y)(z),
	(cross)(verticalVector)
	(dot)(average)(distance)(dist_sq)(getMin)(selectMin)(getMax)(selectMax)(normalize)(normalization)
	(length)(len_sq)(isNaN)(isOutstanding)(saturation)(l_intp)(absolute)(getMinValue)(getMaxValue)
	(linearNormalize)(linearNormalization)(isZero)
	(luaAddV)(luaSubV)(luaMulF)(luaMulV)(luaMulM)(luaDivF)(luaDivV)(luaInvert)(luaEqual)(luaToString)
	(convert<2>)(convert<4>),
	(float)(float)(float)
)
DEF_LUAIMPLEMENT_PTR(
	frea::Vec4, Vec4,
	LUAIMPLEMENT_BASE,
	NOTHING,
	(x)(y)(z)(w),
	(asVec3Coord)
	(dot)(average)(distance)(dist_sq)(getMin)(selectMin)(getMax)(selectMax)(normalize)(normalization)
	(length)(len_sq)(isNaN)(isOutstanding)(saturation)(l_intp)(absolute)(getMinValue)(getMaxValue)
	(linearNormalize)(linearNormalization)(isZero)
	(luaAddV)(luaSubV)(luaMulF)(luaMulV)(luaMulM)(luaDivF)(luaDivV)(luaInvert)(luaEqual)(luaToString)
	(convert<2>)(convert<3>),
	(float)(float)(float)(float)
)
DEF_LUAIMPLEMENT_DERIVED(frea::AVec2, frea::Vec2)
DEF_LUAIMPLEMENT_DERIVED(frea::AVec3, frea::Vec3)
DEF_LUAIMPLEMENT_DERIVED(frea::AVec4, frea::Vec4)
#include "frea/matrix.hpp"
DEF_LUAIMPLEMENT_PTR_NOCTOR(
	frea::Mat2, Mat2,
	LUAIMPLEMENT_BASE,
	(Rotation)(Identity)(Translation)(Scaling),
	NOTHING,
	(rowSwap)(rowMul)(rowMulAdd)(clmSwap)(clmMul)(clmMulAdd)
	(isZeroRow)(isZero)(linearNormalize)(linearNormalization)(isEchelon)(rowReduce)
	(luaAddF)(luaAddM)(luaSubF)(luaSubM)(luaMulF)(luaMulM)(luaMulV)(luaDivF)(luaInvert)(luaToString)
)
DEF_LUAIMPLEMENT_PTR_NOCTOR(
	frea::Mat3, Mat3,
	LUAIMPLEMENT_BASE,
	(RotationX)(RotationY)(RotationZ)(RotationAxis)
	(Identity)(Translation)(Scaling),
	NOTHING,
	(cutRC)(rowSwap)(rowMul)(rowMulAdd)(clmSwap)(clmMul)(clmMulAdd)
	(isZeroRow)(isZero)(linearNormalize)(linearNormalization)(isEchelon)(rowReduce)
	(luaAddF)(luaAddM)(luaSubF)(luaSubM)(luaMulF)(luaMulM)(luaMulV)(luaDivF)(luaInvert)(luaToString)
)
DEF_LUAIMPLEMENT_PTR_NOCTOR(
	frea::Mat4, Mat4,
	LUAIMPLEMENT_BASE,
	(LookAt)(LookDir)(PerspectiveFov)
	(Identity)(Translation)(Scaling),
	NOTHING,
	(cutRC)(rowSwap)(rowMul)(rowMulAdd)(clmSwap)(clmMul)(clmMulAdd)
	(isZeroRow)(isZero)(linearNormalize)(linearNormalization)(isEchelon)(rowReduce)
	(luaAddF)(luaAddM)(luaSubF)(luaSubM)(luaMulF)(luaMulM)(luaMulV)(luaDivF)(luaInvert)(luaToString)
)
DEF_LUAIMPLEMENT_DERIVED(frea::AMat2, frea::Mat2)
DEF_LUAIMPLEMENT_DERIVED(frea::AMat3, frea::Mat3)
DEF_LUAIMPLEMENT_DERIVED(frea::AMat4, frea::Mat4)

#include "frea/expquat.hpp"
#include "frea/quaternion.hpp"
DEF_LUAIMPLEMENT_PTR(
	frea::Quat, Quat,
	LUAIMPLEMENT_BASE,
	(Identity)(FromAxisF)(FromMat)(FromAxis)(FromMatAxis)(RotationYPR)(RotationX)(RotationY)(RotationZ)(LookAt)(SetLookAt)
	(Lua_Rotation)(Lua_RotationFromTo),
	(x)(y)(z)(w),
	(rotateX)(rotateY)(rotateZ)(rotate)(identity)(normalize)(conjugate)(invert)(scale)
	(rotationX)(rotationY)(rotationZ)(rotation)(asVec4)(normalization)(conjugation)(inversion)(len_sq)(length)
	(angle)(getVector)(getAxis)(dot)(slerp)
	(getXAxis)(getXAxisInv)(getYAxis)(getYAxisInv)(getZAxis)(getZAxisInv)
	(getRight)(getUp)(getDir)(asMat33)(asMat44)(distance)(asExpQuat)
	(luaRotation)(luaRotationFromTo)(luaAddQ)(luaSubQ)(luaMulQ)(luaMulF)(luaDivF)(luaEqual)(luaToString),
	(float)(float)(float)(float)
)
DEF_LUAIMPLEMENT_DERIVED(frea::AQuat, frea::Quat)

DEF_LUAIMPLEMENT_PTR(
	frea::ExpQuat, EQuat,
	LUAIMPLEMENT_BASE,
	NOTHING,
	(x)(y)(z),
	(asQuat)(len_sq)(length)(getAngAxis)
	(luaEqual)(luaAddQ)(luaAddF)(luaSubQ)(luaSubF)(luaMulQ)(luaMulF)(luaDivQ)(luaDivF)(luaAsVec3)(luaToString),
	(float)(float)(float)
)
DEF_LUAIMPLEMENT_DERIVED(frea::AExpQuat, frea::ExpQuat)

#include "frea/angle.hpp"
DEF_LUAIMPLEMENT_PTR(
	frea::DegF, Degree,
	LUAIMPLEMENT_BASE,
	(Rotation),
	NOTHING,
	(set)(get)(distance)(semicircle)(single)(rangeValue)(range)
	(luaAddD)(luaAddR)(luaSubD)(luaSubR)(luaMulF)(luaDivF)(luaInvert)(luaToDegree)(luaToRadian)(luaLessthan)(luaLessequal)(luaEqual)(luaToString),
	(float)
)
DEF_LUAIMPLEMENT_PTR(
	frea::RadF, Radian,
	LUAIMPLEMENT_BASE,
	(Rotation),
	NOTHING,
	(set)(get)(distance)(semicircle)(single)(rangeValue)(range)
	(luaAddD)(luaAddR)(luaSubD)(luaSubR)(luaMulF)(luaDivF)(luaInvert)(luaToDegree)(luaToRadian)(luaLessthan)(luaLessequal)(luaEqual)(luaToString),
	(float)
)

#include "frea/plane.hpp"
DEF_LUAIMPLEMENT_PTR_NOCTOR(
	frea::Plane, Plane,
	LUAIMPLEMENT_BASE,
	(FromPtDir)(FromPts)(ChokePoint)(CrossLine),
	(x)(y)(z)(w),
	(dot)(move)(getNormal)(placeOnPlane)(placeOnPlaneDirDist)(getOrigin)(asVec4)(flip)(crosspoint)
	(luaEqual)(luaMulM)(luaToString)
)
DEF_LUAIMPLEMENT_DERIVED(frea::APlane, frea::Plane)

namespace rev {
	void LuaImport::RegisterFreaClass(LuaState& lsc) {
		RegisterClass<frea::Vec2>(lsc);
		RegisterClass<frea::Vec3>(lsc);
		RegisterClass<frea::Vec4>(lsc);
		RegisterClass<frea::AVec2>(lsc);
		RegisterClass<frea::AVec3>(lsc);
		RegisterClass<frea::AVec4>(lsc);

		RegisterClass<frea::Mat2>(lsc);
		RegisterClass<frea::Mat3>(lsc);
		RegisterClass<frea::Mat4>(lsc);
		RegisterClass<frea::AMat2>(lsc);
		RegisterClass<frea::AMat3>(lsc);
		RegisterClass<frea::AMat4>(lsc);

		RegisterClass<frea::Quat>(lsc);
		RegisterClass<frea::ExpQuat>(lsc);
		RegisterClass<frea::DegF>(lsc);
		RegisterClass<frea::RadF>(lsc);
		RegisterClass<frea::Plane>(lsc);
	}
}
