#include "pose3d.hpp"
#include "frea/affine_parts.hpp"
#include "lubee/tostring.hpp"

namespace rev {
	// ------------------------------ Pose3D ------------------------------
	bool Pose3D::_refresh(Accum::value_t& dst, Accum*) const {
		getOffset();
		getRotation();
		getScaling();
		++dst;
		return true;
	}
	bool Pose3D::_refresh(ToWorld::value_t& dst, ToWorld*) const {
		auto& sc = getScaling();
		dst = AMat4::Scaling({sc.x, sc.y, sc.z, 1});
		dst *= getRotation().asMat44();
		dst.setRow<3>(getOffset().convertI<4,3>(1));
		return true;
	}
	bool Pose3D::_refresh(ToLocal::value_t& dst, ToLocal*) const {
		dst = getToWorld().inversion();
		return true;
	}
	Pose3D::Pose3D(const AVec3& pos, const AQuat& rot, const AVec3& sc) {
		setAll(pos, rot, sc);
		setAccum(std::rand());
	}
	Pose3D::Pose3D(const AMat4& m) {
		const auto ap = frea::AffineParts<float>::Decomp(m);
		setAll(ap.offset, ap.rotation, ap.scale);
		setAccum(std::rand());
	}
	void Pose3D::identity() {
		setAll(AVec3(0), AQuat::Identity(), AVec3(1));
	}
	frea::AVec3 Pose3D::getUp() const {
		return getRotation().getUp();
	}
	frea::AVec3 Pose3D::getRight() const {
		return getRotation().getRight();
	}
	frea::AVec3 Pose3D::getDir() const {
		return getRotation().getDir();
	}
	void Pose3D::setAll(const AVec3& ofs, const AQuat& q, const AVec3& sc) {
		setOffset(ofs);
		setRotation(q);
		setScaling(sc);
	}
	void Pose3D::addAxisRotation(const AVec3& axis, const RadF radf) {
		setRotation(getRotation().rotation(axis, radf));
	}
	void Pose3D::addOffset(const AVec3& ad) {
		const AVec3& ofs = getOffset();
		setOffset(ad + ofs);
	}
	void Pose3D::moveFwd2D(const float speed) {
		Vec3 vZ = getDir();
		vZ.y = 0;
		vZ.normalize();
		addOffset(vZ * speed);
	}
	void Pose3D::moveSide2D(const float speed) {
		Vec3 vX = getRight();
		vX.y = 0;
		vX.normalize();
		addOffset(vX * speed);
	}
	void Pose3D::moveFwd3D(const float speed) {
		addOffset(getDir() * speed);
	}
	void Pose3D::moveSide3D(const float speed) {
		addOffset(getRight() * speed);
	}
	void Pose3D::turnAxis(const AVec3& axis, const RadF ang) {
		auto q = getRotation();
		q.rotate(axis, ang);
		setRotation(q);
	}
	void Pose3D::turnYPR(const RadF yaw, const RadF pitch, const RadF roll) {
		auto q = getRotation();
		q *= AQuat::RotationYPR(yaw, pitch, roll);
		setRotation(q);
	}
	void Pose3D::addRotation(const AQuat& q) {
		auto q0 = getRotation();
		q0 *= q;
		setRotation(q0);
	}
	bool Pose3D::lerpTurn(const AQuat& q_tgt, const float t, const float threshold) {
		auto& q = refRotation();
		q.slerp(q_tgt, t);
		return q.distance(q_tgt) < threshold;
	}
	void Pose3D::adjustNoRoll() {
		// X軸のY値が0になればいい

		// 回転を一旦行列に直して軸を再計算
		const auto& q = getRotation();
		auto rm = q.asMat33();
		// Zはそのままに，X軸のY値を0にしてY軸を復元
		AVec3 zA = rm.getRow<2>(),
			xA = rm.getRow<0>();
		xA.y = 0;
		if(xA.len_sq() < 1e-5f) {
			// Xが真上か真下を向いている
			frea::DegF ang;
			if(rm.m[0][1] > 0) {
				// 真上 = Z軸周りに右へ90度回転
				ang = frea::DegF(90);
			} else {
				// 真下 = 左へ90度
				ang = frea::DegF(-90);
			}
			setRotation(AQuat::RotationZ(ang) * q);
		} else {
			xA.normalize();
			AVec3 yA = zA % xA;
			setRotation(AQuat::FromAxis(xA, yA, zA));
		}
	}
	Pose3D& Pose3D::operator = (const AMat4& m) {
		return *this = Pose3D(m);
	}
	bool Pose3D::operator == (const Pose3D& ps) const noexcept {
		return getOffset() == ps.getOffset() &&
				getRotation() == ps.getRotation() &&
				getScaling() == ps.getScaling();
	}
	bool Pose3D::operator != (const Pose3D& ps) const noexcept {
		return !(this->operator == (ps));
	}
	Pose3D Pose3D::lerp(const Pose3D& p1, const float t) const {
		Pose3D ret;
		ret.setOffset(getOffset().l_intp(p1.getOffset(), t));
		ret.setRotation(getRotation().slerp(p1.getRotation(), t));
		ret.setScaling(getScaling().l_intp(p1.getScaling(), t));
		ret.setAccum(p1.getAccum()-1);
		return ret;
	}
	bool Pose3D::equal(const Pose3D& p) const noexcept {
		return *this == p;
	}
	std::string Pose3D::toString() const {
		return lubee::ToString(*this);
	}
	std::ostream& operator << (std::ostream& os, const Pose3D& ps) {
		return os << "Pose3D [ offset:" << ps.getOffset() << std::endl
				<< "rotation: " << ps.getRotation() << std::endl
				<< "scale: " << ps.getScaling() << ']';
	}
}
