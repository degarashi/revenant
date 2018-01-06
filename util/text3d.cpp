#include "text3d.hpp"
#include "sys_unif.hpp"
#include "../camera3d.hpp"

namespace rev {
	namespace util {
		// ---------------------- Text3D ----------------------
		Text3D::Text3D(const float lh, const bool bBillboard):
			_lineHeight(lh),
			_bBillboard(bBillboard)
		{}
		void Text3D::setLineHeight(const float lh) {
			_lineHeight = lh;
		}
		void Text3D::setBillboard(const bool b) {
			_bBillboard = b;
		}
		int Text3D::draw(IEffect& e, const bool bRefresh) const {
			auto& su3d = e.ref3D();
			const auto cid = getCCoreId();
			const float s = float(_lineHeight) / cid.at<CCoreID::Height>();
			auto mScale = frea::AMat4::Scaling({s, s, s, 1});
			mScale *= getToWorld().convert<4,4>();
			return Text::draw(
					e,
					[&,bRefresh](auto&){
						frea::AMat4 m;
						if(_bBillboard) {
							// Poseの位置とスケーリングだけ取って
							// 向きはカメラに正対するように補正
							// Y軸は上
							const auto& pose = su3d.getCamera()->getPose();
							const auto& sc = getScaling();
							auto m0 = frea::Mat4::Scaling({sc.x, sc.y, 1, 1});
							auto m1 = frea::Mat4::LookDir(frea::Vec3(0), pose.getDir(), pose.getUp());
							auto m2 = m1.transposition();
							m = mScale * m0 * m2;
						} else
							m = mScale * getToWorld().convert<4,4>();
						su3d.setWorld(m);
						if(bRefresh)
							su3d.outputUniforms(e.refUniformEnt());
					}
			);
		}
	}
}
