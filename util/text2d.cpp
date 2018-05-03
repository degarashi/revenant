#include "text2d.hpp"
#include "../glx_if.hpp"
#include "frea/matrix.hpp"
#include "lubee/compare.hpp"
#include "../sys_uniform_value.hpp"
#include "gle_nest.hpp"
#include "drawtoken/make_uniform.hpp"
#include "../sys_uniform.hpp"

namespace rev {
	namespace util {
		// ---------------------- Text2D ----------------------
		Text2D::Text2D(const float lh):
			_lineHeight(lh),
			_depth(1)
		{}
		void Text2D::setLineHeight(const float lh) {
			_lineHeight = lh;
		}
		void Text2D::setDepth(const float d) {
			_depth = d;
		}
		int Text2D::draw(IEffect& e, const bool bRefresh) const {
			auto& su2d = dynamic_cast<SystemUniform2D&>(e);
			const auto cid = getCCoreId();
			// Zが0.0未満や1.0以上だと描画されないので、それより少し狭い範囲でクリップする
			const float d = lubee::Saturate(_depth, 0.f, 1.f-1e-4f);
			const float s = float(_lineHeight) / cid.at<CCoreID::Height>();
			auto m = frea::Mat3::Scaling({s, s, 1});
			m *= getToWorld().convert<3,3>();
			return Text::draw(
					e,
					[d, &u=e.refUniformEnt(), &su2d, &m, bR=bRefresh](auto&){
						u.setUniform(unif2d::Depth, d);
						su2d.setWorld(m);
						if(bR) {
							su2d.outputUniforms(u);
						}
					}
			);
		}
	}
}
