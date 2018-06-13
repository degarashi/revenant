#include "font_common.hpp"
#include "gl_resource.hpp"

namespace rev {
	// ------------------- CCoreID -------------------
	CCoreID::CCoreID(const int w, const int h, const uint32_t charFlag, const bool bItalic, const int weightID,
			const CharIDDef::SizeTypeT sizeType, const int faceID)
	{
		at<Width>() = w;
		at<Height>() = h;
		at<CharFlag>() = charFlag;
		at<Italic>() = static_cast<int>(bItalic);
		at<Weight>() = weightID;
		at<FaceID>() = faceID;
		at<SizeType>() = sizeType;
	}
	// ------------------- CharID -------------------
	CharID::CharID(const char32_t ccode, const CCoreID coreID):
		CCoreID(coreID),
		code(ccode)
	{}
	CharID::CharID(const char32_t ccode, const int w, const int h, const int faceID,
			const CharIDDef::CharFlagT flag, const bool bItalic, const int weightID, const CharIDDef::SizeTypeT sizeType):
		CCoreID(w, h, flag, bItalic, weightID, sizeType, faceID),
		code(ccode)
	{}
	uint64_t CharID::get64Bit() const {
		uint64_t val = code;
		val <<= 32;
		val |= cleanedValue();
		return val;
	}
	bool CharID::operator == (const CharID& cid) const {
		return get64Bit() == cid.get64Bit();
	}
	bool CharID::operator != (const CharID& cid) const {
		return !(this->operator==(cid));
	}
	bool CharID::operator < (const CharID& cid) const {
		return get64Bit() < cid.get64Bit();
	}
	// ------------------- CharPlane -------------------
	CharPlane::CharPlane(const lubee::PowSize& size, const int fh, LaneAlloc_UP&& a):
		_sfcSize(size),
		_fontH(fh),
		_lalloc(std::move(a)),
		_nUsed(0)
	{
		_dV = static_cast<float>(fh) / _sfcSize.height;
		_nH = static_cast<int>(1.f / _dV);
	}
	void CharPlane::_addCacheTex() {
		// OpenGLテクスチャ確保
		// MEMO: 環境によってはGL_RGBAが32bitでないかもしれないので対策が必要
		auto hTex = mgr_gl.createTexture(_sfcSize, GL_RGBA, true, true);	// DeviceLost時: 内容のリストア有りで初期化
		// Lane登録
		lubee::RectI rect(0,_sfcSize.width, 0,_fontH);
		for(int i=0 ; i<_nH ; i++) {
			_lalloc->addFreeLane(hTex, rect);
			rect.addOffset(0, _fontH);
		}
	}
	void CharPlane::rectAlloc(LaneRaw& dst, const int width) {
		++_nUsed;
		if(!_lalloc->alloc(dst, width)) {
			// 新しく領域を確保して再トライ
			_addCacheTex();
			Assert0(_lalloc->alloc(dst, width));
		}
	}
	const lubee::PowSize& CharPlane::getSurfaceSize() const {
		return _sfcSize;
	}
	// ------------------- Lane-------------------
	Lane::Lane(const HTexMem& hT, const lubee::RectI& r):
		LaneRaw{hT, r}
	{}
}
