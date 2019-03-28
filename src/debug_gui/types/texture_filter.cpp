#include "../../gl/texture/filter.hpp"
#include "../entry_field.hpp"

namespace rev {
	bool TextureFilter::property(const bool edit) {
		auto field = debug::EntryField("TextureFilter", edit);
		{
			const auto p0 = debug::MakeEditProxy<bool>(_iLinearMag);
			const auto p1 = debug::MakeEditProxy<bool>(_iLinearMin);
			field.entry("Linear-Mag", p0);
			field.entry("Linear-Min", p1);
		}
		field.entry("Aniso-Coeff", _coeff);
		if(field.entry("Wrap-S", _wrapS) | field.entry("Wrap-T", _wrapT))
			setUVWrap(_wrapS, _wrapT);
		field.show("MipLinear", _mipLinear);
		return field.modified();
	}
	const char* TextureFilter::getDebugName() const noexcept {
		return "TextureFilter";
	}
}
