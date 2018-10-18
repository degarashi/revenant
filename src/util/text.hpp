#pragma once
#include "spine/src/optional.hpp"
#include "lubee/src/size.hpp"
#include "font_common.hpp"
#include "../color.hpp"
#include "../abstbuffer.hpp"
#include "../handle/text.hpp"

namespace rev {
	struct DrawTag;
	class IEffect;
	namespace util {
		enum class Align {
			Negative,
			Positive,
			Middle
		};
		using Size_OP = spi::Optional<lubee::SizeI>;
		using CCoreID_OP = spi::Optional<CCoreID>;
		class Text {
			private:
				static CCoreID_OP	cs_defaultCid;
				std::u32string		_text;
				CCoreID				_charId;
				RGBAColor			_color;
				mutable bool		_bRefl;
				mutable HText		_hText;

				static CCoreID _GetDefaultCID();
				using CBPreDraw = std::function<void (IEffect&)>;
			public:
				Text();
				void setCCoreId(CCoreID cid) noexcept;
				const HText& getText() const;
				CCoreID getCCoreId() const noexcept;
				void setText(To32Str str);
				void setText(const HText& h) noexcept;
				void setColor(const RGBAColor& c) noexcept;
				RGBAColor& refColor() noexcept;
				int draw(IEffect& e, const CBPreDraw& cbPre=[](auto&){}) const;
				void exportDrawTag(DrawTag& d) const;
		};
	}
}
