#include "text.hpp"
#include "../font.hpp"
#include "../glx_if.hpp"
#include "uniform_ent.hpp"
#include "../u_common.hpp"

namespace rev {
	namespace util {
		// ---------------------- Text ----------------------
		CCoreID_OP Text::cs_defaultCid;
		CCoreID Text::_GetDefaultCID() {
			if(!cs_defaultCid) {
				cs_defaultCid =
					mgr_text.makeCoreID(
						"IPAGothic",
						CCoreID(
							0,
							20,
							CCoreID::CharFlag_AA,
							false,
							0,
							CCoreID::SizeType_Pixel
						)
					);
			}
			return *cs_defaultCid;
		}
		Text::Text():
			_color(1),
			_bRefl(true)
		{
			_charId = _GetDefaultCID();
		}
		void Text::setCCoreId(const CCoreID cid) noexcept {
			_charId = cid;
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
		CCoreID Text::getCCoreId() const noexcept {
			return _charId;
		}
		const HText& Text::getText() const {
			if(_bRefl) {
				_bRefl = false;
				_hText = mgr_text.createText(_charId, _text);
			}
			return _hText;
		}
		int Text::draw(IEffect& e, const CBPreDraw& cbPre) const {
			getText();

			cbPre(e);
			auto& c = dynamic_cast<U_Common&>(e);
			c.color = _color.asVec4();
			_hText->draw(e);
			return _hText->getSize().width;
		}
		void Text::exportDrawTag(DrawTag& d) const {
			getText()->exportDrawTag(d);
		}
	}
}
