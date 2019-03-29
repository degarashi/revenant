#include "text2d.hpp"
#include "gle_nest.hpp"
#include "../effect/if.hpp"
#include "../effect/uniform/matrix2d.hpp"
#include "../effect/uniform/common.hpp"
#include "frea/src/matrix.hpp"
#include "lubee/src/compare.hpp"

namespace rev {
	namespace drawutil {
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
		int Text2D::draw(IEffect& e) const {
			const auto fid = getFontId();
			auto& c = dynamic_cast<U_Common&>(e);
			// Zが0.0未満や1.0以上だと描画されないので、それより少し狭い範囲でクリップする
			c.depth = lubee::Saturate(_depth, 0.f, 1.f-1e-4f);
			const float s = float(_lineHeight) / fid.at<FontId::Height>();
			auto m = frea::Mat3::Scaling({s, s, 1});
			m *= getToWorld().convert<3,3>();
			return Text::draw(
					e,
					[&e, &m](auto&){
						dynamic_cast<U_Matrix2D&>(e).setWorld(m);
					}
			);
		}
	}
}
