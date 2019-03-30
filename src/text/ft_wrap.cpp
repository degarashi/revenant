#include "ft_wrap.hpp"
#include "ft_error.hpp"
#include "../sdl/rw.hpp"

namespace rev {
	// ---------------------- FTLibrary ----------------------
	FTLibrary::FTLibrary() {
		FTAssert(FT_Init_FreeType, &_lib);
	}
	FTLibrary::~FTLibrary() {
		if(_lib)
			D_FTAssert(FT_Done_FreeType, _lib);
	}
	HFT FTLibrary::newFace(const HRW& hRW, const int index) {
		FT_Face face;
		HRW ret;
		// 中身がメモリじゃなければ一旦コピーする
		if(!hRW->isMemory())
			ret = mgr_rw.fromVector(hRW->readAll());
		else
			ret = hRW;

		auto m = ret->getMemoryC();
		FTAssert(FT_New_Memory_Face, _lib, reinterpret_cast<const uint8_t*>(m.data), m.length, index, &face);
		return emplace(face, ret);
	}

	// ---------------------- FTFace ----------------------
	FTFace::FTFace(const FT_Face face, const HRW& hRW):
		_face(face),
		_hRW(hRW)
	{}
	FTFace::FTFace(FTFace&& f):
		_face(f._face),
		_hRW(std::move(f._hRW)),
		_finfo(f._finfo),
		_info(f._info)
	{
		f._face = nullptr;
	}
	FTFace::~FTFace() {
		if(_face)
			D_FTAssert(FT_Done_Face, _face);
	}
	// met.width>>6 == bitmap.width
	// met.height>>6 == bitmap.height
	// met.horiBearingX>>6 == bitmap_left
	// met.horiBearingY>>6 == bitmap_top
	// 	assert(_info.width == _info.bmp_width &&
	// 			_info.height == _info.bmp_height &&
	// 		  _info.horiBearingX == _info.bmp_left &&
	// 		  _info.horiBearingY == _info.bmp_top);
	void FTFace::prepareGlyph(const char32_t code, const RenderMode::e mode, const bool bBold, const bool bItalic) {
		uint32_t gindex = FT_Get_Char_Index(_face, code);
		int loadflag = mode==RenderMode::Mono ? FT_LOAD_MONOCHROME : FT_LOAD_DEFAULT;
		if(bBold || bItalic)
			loadflag |= FT_LOAD_NO_BITMAP;
		FTAssert(FT_Load_Glyph, _face, gindex, loadflag);

		auto* slot = _face->glyph;
		if(!bBold && !bItalic) {
			if(slot->format != FT_GLYPH_FORMAT_BITMAP)
				FTAssert(FT_Render_Glyph, slot, static_cast<FT_Render_Mode>(mode));
		} else {
			if(bBold) {
				int strength = 1 << 6;
				FTAssert(FT_Outline_Embolden, &slot->outline, strength);
				FTAssert(FT_Render_Glyph, slot, static_cast<FT_Render_Mode>(mode));
			}
			if(bItalic) {
				FT_Matrix mat;
				mat.xx = 1 << 16;
				mat.xy = 0x5800;
				mat.yx = 0;
				mat.yy = 1 << 16;
				FT_Outline_Transform(&slot->outline, &mat);
			}
		}
		Assert0(slot->format == FT_GLYPH_FORMAT_BITMAP);

		auto& met = slot->metrics;
		auto& bm = slot->bitmap;
		_info.data = bm.buffer;
		_info.advanceX = slot->advance.x >> 6;
		_info.nlevel = mode==RenderMode::Mono ? 2 : bm.num_grays;
		_info.pitch = bm.pitch;
		_info.height = bm.rows;
		_info.width = bm.width;
		_info.horiBearingX = met.horiBearingX>>6;
		_info.horiBearingY = met.horiBearingY>>6;
	}
	const FTFace::Info& FTFace::getGlyphInfo() const {
		return _info;
	}
	const FTFace::FInfo& FTFace::getFaceInfo() const {
		return _finfo;
	}
	void FTFace::setPixelSizes(const int w, const int h) {
		FTAssert(FT_Set_Pixel_Sizes, _face, w, h);
		_updateFaceInfo();
	}
	void FTFace::setCharSize(const int w, const int h, const int dpW, const int dpH) {
		FTAssert(FT_Set_Char_Size, _face, w<<6, h<<6, dpW, dpH);
		_updateFaceInfo();
	}
	void FTFace::setSizeFromLine(const int lineHeight) {
		FT_Size_RequestRec req;
		req.height = lineHeight<<6;
		req.width = 0;
		req.type = FT_SIZE_REQUEST_TYPE_CELL;
		req.horiResolution = 0;
		req.vertResolution = 0;
		FTAssert(FT_Request_Size, _face, &req);
		_updateFaceInfo();
	}
	void FTFace::_updateFaceInfo() {
		auto& met = _face->size->metrics;
		_finfo.baseline = (_face->height + _face->descender) *
				met.y_ppem / _face->units_per_EM;
		_finfo.height = met.height >> 6;
		_finfo.maxWidth = met.max_advance >> 6;
	}
	const char* FTFace::getFamilyName() const {
		return _face->family_name;
	}
	const char* FTFace::getStyleName() const {
		return _face->style_name;
	}
	int FTFace::getNFace() const {
		return _face->num_faces;
	}
	int FTFace::getFaceIndex() const {
		return _face->face_index;
	}
}
