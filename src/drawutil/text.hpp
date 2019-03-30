#pragma once
#include "spine/src/optional.hpp"
#include "lubee/src/size.hpp"
#include "../text/common.hpp"
#include "../color.hpp"
#include "../abstbuffer.hpp"
#include "../handle/text.hpp"

namespace rev {
	struct DrawTag;
	class IEffect;
	namespace drawutil {
		enum class Align {
			Negative,
			Positive,
			Middle
		};
		using Size_OP = spi::Optional<lubee::SizeI>;
		using FontId_O = spi::Optional<FontId>;
		class Text {
			private:
				static FontId_O		cs_defaultFid;
				std::u32string		_text;
				FontId				_fontId;
				RGBAColor			_color;
				mutable bool		_bRefl;
				mutable HText		_hText;

				static FontId _GetDefaultFId();
				using CBPreDraw = std::function<void (IEffect&)>;
			public:
				Text();
				void setFontId(FontId fid) noexcept;
				const HText& getText() const;
				FontId getFontId() const noexcept;
				void setText(To32Str str);
				void setText(const HText& h) noexcept;
				void setColor(const RGBAColor& c) noexcept;
				RGBAColor& refColor() noexcept;
				int draw(IEffect& e, const CBPreDraw& cbPre=[](auto&){}) const;
				void exportDrawTag(DrawTag& d) const;
		};
	}
}
