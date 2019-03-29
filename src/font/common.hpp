#pragma once
#include "lubee/src/bitfield.hpp"
#include "lubee/src/rect.hpp"
#include "lubee/src/pow_value.hpp"
#include "handle/opengl.hpp"
#include <vector>

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
	//! フォントのサイズやAAの有無を示す値
	struct FontId : lubee::BitField<CharIdDef> {
		FontId() = default;
		FontId(const FontId& id) = default;
		FontId(int w, int h, uint32_t charFlag, bool bItalic, int weightID, CharIdDef::SizeTypeT sizeType, int faceID=-1);
	};
	//! FontId + 文字コード(UCS4)
	struct CharId : FontId {
		char32_t	code;

		CharId() = default;
		CharId(const CharId& id) = default;
		CharId(char32_t ccode, FontId fontId);
		CharId(char32_t ccode, int w, int h, int faceID, CharIdDef::CharFlagT flag, bool bItalic, int weightID, CharIdDef::SizeTypeT sizeType);

		uint64_t get64Bit() const;
		bool operator == (const CharId& cid) const;
		bool operator != (const CharId& cid) const;
		bool operator < (const CharId& cid) const;
	};
	struct FontName : std::string {
		using std::string::string;
		FontName() = default;
		FontName(const std::string& s): std::string(s) {}
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
	struct hash<::rev::FontName> {
		uint32_t operator()(const ::rev::FontName& fn) const {
			// MSBを必ず1にする
			return hash<std::string>()(static_cast<const std::string&>(fn)) | 0x80000000;
		}
	};
	template <>
	struct hash<::rev::FontId> {
		uint32_t operator()(const ::rev::FontId& fid)	const {
			return fid.cleanedValue();
		}
	};
}
namespace rev {
	struct LaneRaw {
		HTexMem2D		hTex;
		lubee::RectI	rect;	//!< 管理している領域
	};
	struct Lane : LaneRaw {
		Lane			*pNext;
		Lane(const HTexMem2D& hT, const lubee::RectI& r);
	};
	struct ILaneAlloc {
		virtual ~ILaneAlloc() {}
		virtual bool alloc(LaneRaw& dst, std::size_t w) = 0;
		virtual void addFreeLane(const HTexMem2D& hTex, const lubee::RectI& rect) = 0;
		virtual void clear() = 0;
	};
	using LaneAlloc_UP = std::unique_ptr<ILaneAlloc>;
	//! CharPlaneと、その位置
	struct CharPos {
		HTexSrcC		hTex;		//!< フォントが格納されているテクスチャ (ハンドル所有権は別途CharPlaneが持つ)
		lubee::RectF	uv;			//!< 参照すべきUV値
		lubee::RectI	box;		//!< フォント原点に対する相対描画位置 (サイズ)
		int				space;		//!< カーソルを進めるべき距離
	};
	//! フォントのGLテクスチャ
	/*! 縦幅は固定。横は必要に応じて確保 */
	class CharPlane {
		private:
			lubee::PowSize	_sfcSize;
			const int		_fontH;		//!< フォント縦幅 (=height)
			LaneAlloc_UP	_lalloc;	//!< レーンの残り幅管理
			int				_nUsed;		//!< 割り当て済みのChar数(動作には影響しない)
			int				_nH;		//!< Plane一枚のLane数
			float			_dV;		//!< 1文字のVサイズ

			//! キャッシュテクスチャを一枚追加 -> Lane登録
			void _addCacheTex();

		public:
			//! フォントキャッシュテクスチャの確保
			/*! \param size[in] テクスチャ1辺のサイズ
				\param fh[in] Char高 */
			CharPlane(const lubee::PowSize& size, int fh, LaneAlloc_UP&& a);
			CharPlane(CharPlane&& cp) = default;
			//! 新しいChar登録領域を確保
			/*! まだどこにも登録されてないcodeである事はFontArray_Depが保証する
				\param[out] dst uv, hTexを書き込む */
			void rectAlloc(LaneRaw& dst, int width);
			const lubee::PowSize& getSurfaceSize() const;
	};
}
