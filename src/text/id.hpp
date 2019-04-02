#pragma once
#include "lubee/src/bitfield.hpp"

namespace rev {
	//! キャラクタの属性値を32bitで表す
	struct CharIdDef :
		lubee::BitDef<
			uint32_t,
			lubee::BitF<0,8>, // Width
			lubee::BitF<8,8>, // Height
			lubee::BitF<16,8>, // FaceId
			lubee::BitF<24,2>, // Flag
			lubee::BitF<26,1>, // Italic
			lubee::BitF<27,3>, // Weight
			lubee::BitF<30,2> // SizeType
		>
	{
		enum { Width, Height, FaceId, CharFlag, Italic, Weight, SizeType };
		enum CharFlagT {
			CharFlag_AA = 0x01,
			CharFlag_Hemming = 0x02
		};
		//! フォントのサイズ指定方法
		enum SizeTypeT {
			SizeType_Pixel,			//!< ピクセル単位の指定
			SizeType_Point,			//!< DPIに相対したポイント指定
			SizeType_LineHeight		//!< ラインに収まるサイズ
		};
	};
	// ビットフィールド用のUnsignedな値
	using EnumUInt = uint_fast32_t;
	//! フォントのサイズやAAの有無を示す値
	struct FontId :
		lubee::BitField<CharIdDef>
	{
		FontId() = default;
		FontId(const FontId& id) = default;
		FontId(
			EnumUInt w,
			EnumUInt h,
			CharFlagT charFlag,
			bool bItalic,
			EnumUInt weightID,
			SizeTypeT sizeType,
			EnumUInt faceID = InvalidFaceId
		);
		const static EnumUInt InvalidFaceId;
	};
	//! FontId + 文字コード(UCS4)
	struct CharId : FontId {
		char32_t	code;

		CharId() = default;
		CharId(const CharId& id) = default;
		CharId(char32_t ccode, FontId fontId);
		CharId(
			char32_t ccode,
			EnumUInt w,
			EnumUInt h,
			EnumUInt faceID,
			CharFlagT flag,
			bool bItalic,
			EnumUInt weightID,
			SizeTypeT sizeType
		);

		uint64_t get64Bit() const;
		bool operator == (const CharId& cid) const;
		bool operator != (const CharId& cid) const;
		bool operator < (const CharId& cid) const;
	};
}
namespace std {
	template <>
	struct hash<::rev::CharId> {
		uint32_t operator()(const ::rev::CharId& cid) const {
			// MSBを必ず0にする
			const uint32_t tmp = cid.code ^ 0x1234abcd;
			return ((cid.cleanedValue() * cid.code) ^ tmp) & 0x7fffffff;
		}
	};
	template <>
	struct hash<::rev::FontId> {
		uint32_t operator()(const ::rev::FontId& fid)	const {
			return fid.cleanedValue();
		}
	};
}
