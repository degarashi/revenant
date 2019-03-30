#include "text.hpp"
#include "../font/font.hpp"
#include "../effect/if.hpp"
#include "../effect/uniform_ent.hpp"
#include "../effect/uniform/common.hpp"

namespace rev {
	namespace drawutil {
		// ---------------------- Text ----------------------
		FontId_O Text::cs_defaultFid;
		FontId Text::_GetDefaultFId() {
			if(!cs_defaultFid) {
				cs_defaultFid =
					mgr_text.makeFontId(
						"IPAGothic",
						FontId(
							0,
							20,
							FontId::CharFlag_AA,
							false,
							0,
							FontId::SizeType_Pixel
						)
					);
			}
			return *cs_defaultFid;
		}
		Text::Text():
			_color(1),
			_bRefl(true)
		{
			_fontId = _GetDefaultFId();
		}
		void Text::setFontId(const FontId fid) noexcept {
			_fontId = fid;
		}
		void Text::setText(To32Str str) {
			_text = str.moveTo();
			_bRefl = true;
		}
		void Text::setText(const HText& h) noexcept {
			_hText = h;
			_bRefl = false;
		}
		RGBAColor& Text::refColor() noexcept {
			return _color;
		}
		void Text::setColor(const RGBAColor& c) noexcept {
			_color = c;
		}
		FontId Text::getFontId() const noexcept {
			return _fontId;
		}
		const HText& Text::getText() const {
			if(_bRefl) {
				_bRefl = false;
				_hText = mgr_text.createText(_fontId, _text);
			}
			return _hText;
		}
		int Text::draw(IEffect& e, const CBPreDraw& cbPre) const {
			getText();

			cbPre(e);
			auto& c = dynamic_cast<U_Common&>(e);
			c.color = _color.asVec4();
			_hText->draw(e);
			return _hText->getDrawSize().width;
		}
		void Text::exportDrawTag(DrawTag& d) const {
			getText()->exportDrawTag(d);
		}
	}
}
