#include "camera2d.hpp"

namespace rev {
	Camera2D::Camera2D() {
		setAccum(std::rand());
		setAspectRatio(1.f);
	}
	bool Camera2D::_refresh(Accum::value_t& acc, Accum*) const {
		const auto ret = _rflag.getWithCheck(this, acc);
		const bool b = ret.flag;
		if(b) {
			// カウンタをインクリメント
			++acc;
		}
		return b;
	}
	bool Camera2D::_refresh(View::value_t& m, View*) const {
		const auto ret = _rflag.getWithCheck(this, m);
		auto& ps = *std::get<0>(ret);
		const bool b = ret.flag;
		if(b)
			m = ps.getToLocal();
		return b;
	}
	bool Camera2D::_refresh(ViewInv::value_t& m, ViewInv*) const {
		m = getView().inversion();
		return true;
	}
	bool Camera2D::_refresh(Proj::value_t& m, Proj*) const {
		m = AMat3::Scaling({1.f / getAspectRatio(), 1, 1});
		return true;
	}
	bool Camera2D::_refresh(ViewProj::value_t& m, ViewProj*) const {
		m = getView() * getProj();
		return true;
	}
	bool Camera2D::_refresh(ViewProjInv::value_t& m, ViewProjInv*) const {
		m = getViewProj().inversion();
		return true;
	}

	Camera2D::Vec2 Camera2D::vp2w(const Vec2& pos) const {
		return (pos.convert<3>() * getViewProjInv()).convert<2>();
	}
	Camera2D::Vec2 Camera2D::v2w(const Vec2& pos) const {
		return (pos.convertI<3,2>(1) * getViewInv()).convert<2>();
	}
	const char* Camera2D::getResourceName() const noexcept {
		return "Camera2D";
	}

	bool Camera2D::operator == (const Camera2D& c) const noexcept {
		// 最下層だけ比較
		return getPose() == c.getPose() &&
				getAspectRatio() == c.getAspectRatio();
	}
	bool Camera2D::operator != (const Camera2D& c) const noexcept {
		return !(this->operator == (c));
	}
}
