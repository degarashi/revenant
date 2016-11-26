#include "texthud.hpp"
#include "../glx_if.hpp"
#include "../sys_uniform_value.hpp"
#include "../systeminfo.hpp"
#include "lubee/compare.hpp"

namespace rev {
	namespace util {
		// ---------------------- TextHUD ----------------------
		const IdValue TextHUD::U_Text = IEffect::GlxId::GenUnifId("mText");
		TextHUD::TextHUD():
			_coordType(Coord::Window),
			_offset(0),
			_scale(1),
			_depth(1)
		{}
		frea::Mat3 TextHUD::_makeMatrix() const {
			float rx, ry,
				  ox, oy,
				  diry;
			const auto s = mgr_info.getScreenSize();
			rx = 1.f / (s.width/2);
			ry = 1.f / (s.height/2);
			auto ofs = _offset;
			if(_coordType == Coord::Window) {
				ox = -1.f;
				oy = 1.f;
				diry = -1.f;
				ofs.x *= rx;
				ofs.y *= ry;
			} else {
				ox = oy = 0.f;
				diry = 1.f;
			}
			return frea::Mat3(rx*_scale.x,		0,						0,
							0,					ry*_scale.y,			0,
							ox + ofs.x,			oy + ofs.y*diry,		1);
		}
		void TextHUD::setWindowOffset(const frea::Vec2& ofs) {
			_coordType = Coord::Window;
			_offset = ofs;
		}
		void TextHUD::setScreenOffset(const frea::Vec2& ofs) {
			_coordType = Coord::Screen;
			_offset = ofs;
		}
		void TextHUD::setScale(const frea::Vec2& s) {
			_scale = s;
		}
		void TextHUD::setDepth(const float d) {
			_depth = d;
		}
		int TextHUD::draw(IEffect& e) const {
			// Zが0.0未満や1.0以上だと描画されないので、それより少し狭い範囲でクリップする
			const float d = lubee::Saturate(_depth, 0.f, 1.f-1e-4f);
			return Text::draw(
						e,
						[d, this](auto& e){
							e.setUniform(unif2d::Depth, d);
							e.setUniform(U_Text, _makeMatrix(), true);
						}
					);
		}
	}
}
