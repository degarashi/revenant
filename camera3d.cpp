#include "camera3d.hpp"

namespace rev {
	bool Camera3D::_refresh(typename Accum::value_t& acc, Accum*) const {
		const auto ret = _rflag.getWithCheck(this, acc);
		const bool b = ret.flag;
		if(b) {
			// カウンタをインクリメント
			++acc;
		}
		return b;
	}
	bool Camera3D::_refresh(typename View::value_t& m, View*) const {
		auto ret = _rflag.getWithCheck(this, m);
		auto& ps = *std::get<0>(ret);
		const bool b = ret.flag;
		if(b) {
			// 回転を一端quatに変換
			const Quat& q = ps.getRotation();
			m = AMat4::Identity();
			m.getRow<3>() = -ps.getOffset().convert<4>();

			const Quat tq = q.inversion();
			m *= tq.asMat33().convertI<4,4>(1);
		}
		return b;
	}
	bool Camera3D::_refresh(AMat4& m, Proj*) const {
		m = AMat4::PerspectiveFov(getFov(), getAspect(), {getNearZ(), getFarZ()});
		return true;
	}
	bool Camera3D::_refresh(AMat4& m, ViewProj*) const {
		m = getView() * getProj();
		return true;
	}
	bool Camera3D::_refresh(AMat4& m, ViewProjInv*) const {
		m = getViewProj().inversion();
		return true;
	}
	// bool Camera3D::_refresh(Frustum& vf, VFrustum*) const {
	// 	// UP軸とZ軸を算出
	// 	AMat44 mat;
	// 	getView().convertA44().inversion(mat);
	// 	mat.getRow(3) = AVec4(0,0,0,1);
	// 	AVec4 zAxis(0,0,1,0),
	// 		yAxis(0,1,0,0);
	// 	zAxis *= mat;
	// 	yAxis *= mat;
	//
	// 	auto& ps = getPose();
	// 	vf = Frustum(ps.getOffset(), zAxis.asVec3(), yAxis.asVec3(), getFov(), getFarZ(), getAspect());
	// 	return true;
	// }

	Camera3D::Camera3D() {
		refPose().setAll(Vec3(0,0,-10), Quat(0,0,0,1), Vec3(1,1,1));
		setAccum(1);

		setNearZ(1.f);
		setFarZ(1e5f);
		setFov(frea::DegF(90.0f));
		setAspect(1.4f / 1.f);
	}
	void Camera3D::setZPlane(const float n, const float f) {
		setNearZ(n);
		setFarZ(f);
	}
	Camera3D::Vec3 Camera3D::unproject(const Vec3& vsPos) const {
		const AMat4& mI = getViewProjInv();
		return (vsPos.convertI<4,3>(1) * mI).asVec3Coord();
	}
	Camera3D::Vec3x2 Camera3D::unprojectVec(const Vec2& vsPos) const {
		const AMat4& mI = getViewProjInv();
		return Vec3x2(
			(AVec4(vsPos.x, vsPos.y, 0, 1) * mI).asVec3Coord(),
			(AVec4(vsPos.x, vsPos.y, 1, 1) * mI).asVec3Coord()
		);
	}
	Camera3D::Vec3 Camera3D::vp2wp(const Vec3& vp) const {
		return (vp.convertI<4,3>(1) * getViewProjInv()).asVec3Coord();
	}
	Camera3D::Plane Camera3D::getNearPlane() const {
		auto& ps = getPose();
		const Vec3 dir = ps.getDir();
		return Plane::FromPtDir(ps.getOffset() + dir*getNearZ(), dir);
	}
	// カメラの変換手順としては offset -> rotation だがPose3Dの変換は rotation -> offsetなので注意！
	// Frustum Camera3D::getNearFrustum() const {
	// 	Frustum fr;
	// 	float t = std::tan(getFov().get()/2);
	// 	fr.setScale({t*getAspect(), t, getNearZ()*8});
	// 	auto& ps = getPose();
	// 	fr.setRot(ps.getRot());
	// 	fr.setOffset(ps.getOffset());
	// 	return fr;
	// }
	const char* Camera3D::getResourceName() const noexcept {
		return "Camera3D";
	}
	bool Camera3D::operator == (const Camera3D& c) const noexcept {
		// 最下層だけ比較
		return getPose() == c.getPose() &&
				getFov() == c.getFov() &&
				getAspect() == c.getAspect() &&
				getNearZ() == c.getNearZ() &&
				getFarZ() == c.getFarZ();
	}
	bool Camera3D::operator != (const Camera3D& c) const noexcept {
		return !(this->operator == (c));
	}
}

