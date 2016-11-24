#include "luaimpl.hpp"
#ifndef NOTHING
	#define NOTHING
#endif

#include "frea/vector.hpp"
DEF_LUAIMPLEMENT_PTR(
	frea::Vec2, Vec2,
	(x)(y),
	(Ccw)(Cw)
	(dot)(average)(distance)(dist_sq)(getMin)(selectMin)(getMax)(selectMax)(normalize)(normalization)
	(length)(len_sq)(isNaN)(isOutstanding)(saturation)(l_intp)(absolute)(getMinValue)(getMaxValue)
	(linearNormalize)(linearNormalization)(isZero)
	(luaAddV)(luaSubV)(luaMulF)(luaMulM)(luaDivF)(luaInvert)(luaEqual)(luaToString),
	(float)(float)
)
DEF_LUAIMPLEMENT_PTR(
	frea::Vec3, Vec3,
	(x)(y)(z),
	(cross)(verticalVector)
	(dot)(average)(distance)(dist_sq)(getMin)(selectMin)(getMax)(selectMax)(normalize)(normalization)
	(length)(len_sq)(isNaN)(isOutstanding)(saturation)(l_intp)(absolute)(getMinValue)(getMaxValue)
	(linearNormalize)(linearNormalization)(isZero)
	(luaAddV)(luaSubV)(luaMulF)(luaMulM)(luaDivF)(luaInvert)(luaEqual)(luaToString),
	(float)(float)(float)
)
DEF_LUAIMPLEMENT_PTR(
	frea::Vec4, Vec4,
	(x)(y)(z)(w),
	(asVec3Coord)
	(dot)(average)(distance)(dist_sq)(getMin)(selectMin)(getMax)(selectMax)(normalize)(normalization)
	(length)(len_sq)(isNaN)(isOutstanding)(saturation)(l_intp)(absolute)(getMinValue)(getMaxValue)
	(linearNormalize)(linearNormalization)(isZero)
	(luaAddV)(luaSubV)(luaMulF)(luaMulM)(luaDivF)(luaInvert)(luaEqual)(luaToString),
	(float)(float)(float)(float)
)
DEF_LUAIMPLEMENT_DERIVED(frea::AVec2, frea::Vec2)
DEF_LUAIMPLEMENT_DERIVED(frea::AVec3, frea::Vec3)
DEF_LUAIMPLEMENT_DERIVED(frea::AVec4, frea::Vec4)
#include "frea/matrix.hpp"
DEF_LUAIMPLEMENT_PTR_NOCTOR(
	frea::Mat2, Mat2,
	NOTHING,
	(Rotation)
	(Identity)(Translation)(Scaling)
	(rowSwap)(rowMul)(rowMulAdd)(clmSwap)(clmMul)(clmMulAdd)
	(isZeroRow)(isZero)(linearNormalize)(linearNormalization)(isEchelon)(rowReduce)
	(luaAddF)(luaAddM)(luaSubF)(luaSubM)(luaMulF)(luaMulM)(luaMulV)(luaDivF)(luaInvert)(luaToString)
)
DEF_LUAIMPLEMENT_PTR_NOCTOR(
	frea::Mat3, Mat3,
	NOTHING,
	(RotationX)(RotationY)(RotationZ)(RotationAxis)
	(Identity)(Translation)(Scaling)
	(cutRC)(rowSwap)(rowMul)(rowMulAdd)(clmSwap)(clmMul)(clmMulAdd)
	(isZeroRow)(isZero)(linearNormalize)(linearNormalization)(isEchelon)(rowReduce)
	(luaAddF)(luaAddM)(luaSubF)(luaSubM)(luaMulF)(luaMulM)(luaMulV)(luaDivF)(luaInvert)(luaToString)
)
DEF_LUAIMPLEMENT_PTR_NOCTOR(
	frea::Mat4, Mat4,
	NOTHING,
	(LookAt)(LookDir)(PerspectiveFov)
	(Identity)(Translation)(Scaling)
	(cutRC)(rowSwap)(rowMul)(rowMulAdd)(clmSwap)(clmMul)(clmMulAdd)
	(isZeroRow)(isZero)(linearNormalize)(linearNormalization)(isEchelon)(rowReduce)
	(luaAddF)(luaAddM)(luaSubF)(luaSubM)(luaMulF)(luaMulM)(luaMulV)(luaDivF)(luaInvert)(luaToString)
)
DEF_LUAIMPLEMENT_DERIVED(frea::AMat2, frea::Mat2)
DEF_LUAIMPLEMENT_DERIVED(frea::AMat3, frea::Mat3)
DEF_LUAIMPLEMENT_DERIVED(frea::AMat4, frea::Mat4)

#include "frea/expquat.hpp"
#include "frea/quaternion.hpp"
DEF_LUAIMPLEMENT_PTR_NOCTOR(
	frea::Quat, Quat,
	(x)(y)(z)(w),
	(Identity)(FromAxisF)(FromMat)(FromAxis)(FromMatAxis)(RotationYPR)(RotationX)(RotationY)(RotationZ)(LookAt)(SetLookAt)
	(rotateX)(rotateY)(rotateZ)(rotate)(identity)(normalize)(conjugate)(invert)(scale)
	(rotationX)(rotationY)(rotationZ)(rotation)(asVec4)(normalization)(conjugation)(inversion)(len_sq)(length)
	(angle)(getVector)(getAxis)(dot)(slerp)
	(getXAxis)(getXAxisInv)(getYAxis)(getYAxisInv)(getZAxis)(getZAxisInv)
	(getRight)(getUp)(getDir)(asMat33)(asMat44)(distance)(asExpQuat)
	(Lua_Rotation)(Lua_RotationFromTo)
	(luaRotation)(luaRotationFromTo)(luaAddQ)(luaSubQ)(luaMulQ)(luaMulF)(luaDivF)(luaEqual)(luaToString)
)
DEF_LUAIMPLEMENT_DERIVED(frea::AQuat, frea::Quat)

DEF_LUAIMPLEMENT_PTR_NOCTOR(
	frea::ExpQuat, EQuat,
	(x)(y)(z),
	(asQuat)(len_sq)(length)(luaAsVec3)(getAngAxis)
	(luaEqual)(luaAddQ)(luaAddF)(luaSubQ)(luaSubF)(luaMulQ)(luaMulF)(luaDivQ)(luaDivF)(luaAsVec3)(luaToString)
)
DEF_LUAIMPLEMENT_DERIVED(frea::AExpQuat, frea::ExpQuat)

#include "frea/angle.hpp"
DEF_LUAIMPLEMENT_PTR(
	frea::DegF, Degree,
	NOTHING,
	(Rotation)
	(set)(get)(distance)(semicircle)(single)(rangeValue)(range)
	(luaAddD)(luaAddR)(luaSubD)(luaSubR)(luaMulF)(luaDivF)(luaInvert)(luaToDegree)(luaToRadian)(luaLessthan)(luaLessequal)(luaEqual)(luaToString),
	(float)
)
DEF_LUAIMPLEMENT_PTR(
	frea::RadF, Radian,
	NOTHING,
	(Rotation)
	(set)(get)(distance)(semicircle)(single)(rangeValue)(range)
	(luaAddD)(luaAddR)(luaSubD)(luaSubR)(luaMulF)(luaDivF)(luaInvert)(luaToDegree)(luaToRadian)(luaLessthan)(luaLessequal)(luaEqual)(luaToString),
	(float)
)

#include "frea/plane.hpp"
DEF_LUAIMPLEMENT_PTR_NOCTOR(
	frea::Plane, Plane,
	(x)(y)(z)(w),
	(FromPtDir)(FromPts)(ChokePoint)(CrossLine)
	(dot)(move)(getNormal)(placeOnPlane)(placeOnPlaneDirDist)(getOrigin)(asVec4)(flip)(crosspoint)
	(luaEqual)(luaMulM)(luaToString)
)
DEF_LUAIMPLEMENT_DERIVED(frea::APlane, frea::Plane)
