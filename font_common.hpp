#pragma once
#include "lubee/bitfield.hpp"
#include "lubee/rect.hpp"
#include "handle.hpp"
#include <vector>

namespace rev {
	//! キャラクタの属性値を32bitで表す
	struct CharIDDef :
		lubee::BitDef<
			uint32_t,
			lubee::BitF<0,8>, // Width
			lubee::BitF<8,8>, // Height
			lubee::BitF<16,8>, // FaceID
			lubee::BitF<24,2>, // Flag
			lubee::BitF<26,1>, // Italic
			lubee::BitF<27,3>, // Weight
			lubee::BitF<30,2> // SizeType
		>
	{
		enum { Width, Height, FaceID, CharFlag, Italic, Weight, SizeType };
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
	struct CCoreID : lubee::BitField<CharIDDef> {
		CCoreID() = default;
		CCoreID(const CCoreID& id) = default;
		CCoreID(int w, int h, uint32_t charFlag, bool bItalic, int weightID, CharIDDef::SizeTypeT sizeType, int faceID=-1);
	};
	//! CCoreID + 文字コード(UCS4)
	struct CharID : CCoreID {
		char32_t	code;

		CharID() = default;
		CharID(const CharID& id) = default;
		CharID(char32_t ccode, CCoreID coreID);
		CharID(char32_t ccode, int w, int h, int faceID, CharIDDef::CharFlagT flag, bool bItalic, int weightID, CharIDDef::SizeTypeT sizeType);

		uint64_t get64Bit() const;
		bool operator == (const CharID& cid) const;
		bool operator != (const CharID& cid) const;
		bool operator < (const CharID& cid) const;
	};
	struct FontName : std::string {
		using std::string::string;
		FontName() = default;
		FontName(const std::string& s): std::string(s) {}
	};
}
namespace std {
	template <>
	struct hash<::rev::CharID> {
		uint32_t operator()(const ::rev::CharID& cid) const {
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
	struct hash<::rev::CCoreID> {
		uint32_t operator()(const ::rev::CCoreID& cid)	const {
			return cid.cleanedValue();
		}
	};
}
namespace rev {
	struct LaneRaw {
		HTex			hTex;
		lubee::RectI	rect;	//!< 管理している領域
	};
	struct Lane : LaneRaw {
		Lane	*pNext = nullptr;
		Lane(const HTex& hT, const lubee::RectI& r);
	};
	struct ILaneAlloc {
		virtual ~ILaneAlloc() {}
		virtual bool alloc(LaneRaw& dst, std::size_t w) = 0;
		virtual void addFreeLane(const HTex& hTex, const lubee::RectI& rect) = 0;
		virtual void clear() = 0;
	};
	using LaneAlloc_UP = std::unique_ptr<ILaneAlloc>;
	//! CharPlaneと、その位置
	struct CharPos {
		HTex			hTex;		//!< フォントが格納されているテクスチャ (ハンドル所有権は別途CharPlaneが持つ)
		lubee::RectF	uv;			//!< 参照すべきUV値
		lubee::RectI	box;		//!< フォント原点に対する相対描画位置 (サイズ)
		int				space;		//!< カーソルを進めるべき距離
	};
	//! フォントのGLテクスチャ
	/*! 縦幅は固定。横は必要に応じて確保 */
	class CharPlane {
		private:
			using PlaneVec = std::vector<HTex>;
			PlaneVec		_plane;
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
