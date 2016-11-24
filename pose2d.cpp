#include "pose2d.hpp"
#include "lubee/tostring.hpp"
#include "frea/angle_func.hpp"

namespace rev {
	// ------------------------------ Pose2D ------------------------------
	bool Pose2D::_refresh(Accum::value_t& dst, Accum*) const {
		getOffset();
		getRotation();
		getScaling();
		++dst;
		return true;
	}
	bool Pose2D::_refresh(ToWorld::value_t& dst, ToWorld*) const {
		dst = AMat3::Scaling(getScaling().convert<3>());
		dst *= frea::AMat2::Rotation(getRotation()).convertI<3,3,2>(1);
		dst.setRow<2>(getOffset().convertI<3,2>(1));
		return true;
	}
	bool Pose2D::_refresh(ToLocal::value_t& dst, ToLocal*) const {
		dst = getToWorld().inversion();
		return true;
	}
	Pose2D::Pose2D(const Vec2& pos, const RadF ang, const Vec2& sc) {
		setAll(pos, ang, sc);
		setAccum(std::rand());
	}
	void Pose2D::identity() {
		setAll(Vec2(0), RadF(0), Vec2(1));
	}
	void Pose2D::setAll(const Vec2& ofs, const RadF ang, const Vec2& sc) {
		setOffset(ofs);
		setRotation(ang);
		setScaling(sc);
	}
	void Pose2D::moveUp(const float speed) {
		refOffset() += getUp() * speed;
	}
	void Pose2D::moveDown(const float speed) {
		moveUp(-speed);
	}
	void Pose2D::moveRight(const float speed) {
		refOffset() += getRight() * speed;
	}
	void Pose2D::moveLeft(const float speed) {
		moveRight(-speed);
	}
	frea::Vec2 Pose2D::getUp() const {
		const float rd = (getRotation() + frea::DegF(90)).get();
		return {std::cos(rd), std::sin(rd)};
	}
	frea::Vec2 Pose2D::getRight() const {
		const float rd = getRotation().get();
		return {std::cos(rd), std::sin(rd)};
	}
	void Pose2D::setUp(const Vec2& up) {
		setRotation(AngleValue(up));
	}
	Pose2D Pose2D::lerp(const Pose2D& p1, const float t) const {
		Pose2D ret;
		ret.setOffset(getOffset().l_intp(p1.getOffset(), t));
		ret.setRotation((p1.getRotation() - getRotation()) * t + getRotation());
		ret.setScaling(getScaling().l_intp(p1.getScaling(), t));
		ret.setAccum(getAccum()-1);
		return ret;
	}
	bool Pose2D::operator == (const Pose2D& ps) const noexcept {
		return getOffset() == ps.getOffset() &&
				getRotation() == ps.getRotation() &&
				getScaling() == ps.getScaling();
	}
	bool Pose2D::operator != (const Pose2D& ps) const noexcept {
		return !(this->operator == (ps));
	}
	bool Pose2D::equal(const Pose2D& p) const noexcept {
		return *this == p;
	}
	std::string Pose2D::toString() const {
		return lubee::ToString(*this);
	}
	std::ostream& operator << (std::ostream& os, const Pose2D& ps) {
		return os << "Pose2D [ offset: " << ps.getOffset() << std::endl
				<< "angle: " << ps.getRotation() << std::endl
				<< "scale: " << ps.getScaling() << ']';
	}
}
