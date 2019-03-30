#include "id.hpp"

namespace rev {
	// ------------------- FontId -------------------
	FontId::FontId(
		const EnumUInt w,
		const EnumUInt h,
		const CharFlagT charFlag,
		const bool bItalic,
		const EnumUInt weightID,
		const SizeTypeT sizeType,
		const EnumUInt faceID
	) {
		at<Width>() = w;
		at<Height>() = h;
		at<CharFlag>() = charFlag;
		at<Italic>() = static_cast<EnumUInt>(bItalic);
		at<Weight>() = weightID;
		at<FaceId>() = faceID;
		at<SizeType>() = sizeType;
	}
	// ------------------- CharId -------------------
	CharId::CharId(const char32_t ccode, const FontId fontId):
		FontId(fontId),
		code(ccode)
	{}
	CharId::CharId(
		const char32_t ccode,
		const EnumUInt w,
		const EnumUInt h,
		const EnumUInt faceID,
		const CharFlagT flag,
		const bool bItalic,
		const EnumUInt weightID,
		const SizeTypeT sizeType
	):
		FontId(w, h, flag, bItalic, weightID, sizeType, faceID),
		code(ccode)
	{}
	uint64_t CharId::get64Bit() const {
		uint64_t val = code;
		val <<= 32;
		val |= cleanedValue();
		return val;
	}
	bool CharId::operator == (const CharId& cid) const {
		return get64Bit() == cid.get64Bit();
	}
	bool CharId::operator != (const CharId& cid) const {
		return !(this->operator==(cid));
	}
	bool CharId::operator < (const CharId& cid) const {
		return get64Bit() < cid.get64Bit();
	}
}
