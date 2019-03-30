#include "wrap.hpp"
#include "error.hpp"
#include "../../sdl/rw.hpp"

namespace rev {
	namespace {
		constexpr unsigned int FTUnit = 64;
	}
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
		_faceInfo(f._faceInfo),
		_glyphInfo(f._glyphInfo)
	{
		f._face = nullptr;
	}
	FTFace::~FTFace() {
		if(_face)
			D_FTAssert(FT_Done_Face, _face);
	}
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
				int strength = 1 * FTUnit;
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

		/*
			met.width / FTUnit == bitmap.width
			met.height / FTUnit == bitmap.height
			met.horiBearingX / FTUnit == bitmap_left
			met.horiBearingY / FTUnit == bitmap_top
			assert(_info.width == _info.bmp_width &&
					_info.height == _info.bmp_height &&
				  _info.horiBearingX == _info.bmp_left &&
				  _info.horiBearingY == _info.bmp_top);
		*/
		auto& met = slot->metrics;
		auto& bm = slot->bitmap;
		_glyphInfo.data = static_cast<const uint8_t*>(bm.buffer);
		_glyphInfo.advanceX = slot->advance.x / FTUnit;
		_glyphInfo.nlevel = mode==RenderMode::Mono ? 2 : bm.num_grays;
		_glyphInfo.pitch = bm.pitch;
		_glyphInfo.height = bm.rows;
		_glyphInfo.width = bm.width;
		_glyphInfo.horiBearingX = met.horiBearingX / FTUnit;
		_glyphInfo.horiBearingY = met.horiBearingY / FTUnit;
	}
	const FTFace::GlyphInfo& FTFace::getGlyphInfo() const {
		return _glyphInfo;
	}
	const FTFace::FaceInfo& FTFace::getFaceInfo() const {
		return _faceInfo;
	}
	void FTFace::setPixelSizes(const lubee::SizeI s) {
		FTAssert(FT_Set_Pixel_Sizes, _face, s.width, s.height);
		_updateFaceInfo();
	}
	void FTFace::setCharSize(const lubee::SizeI s, const lubee::SizeI dpi) {
		FTAssert(FT_Set_Char_Size,
			_face,
			s.width * FTUnit, s.height * FTUnit,
			dpi.width, dpi.height
		);
		_updateFaceInfo();
	}
	void FTFace::setSizeFromLine(const unsigned int lineHeight) {
		FT_Size_RequestRec req;
		req.height = lineHeight * FTUnit;
		req.width = 0;
		req.type = FT_SIZE_REQUEST_TYPE_CELL;
		req.horiResolution = 0;
		req.vertResolution = 0;
		FTAssert(FT_Request_Size, _face, &req);
		_updateFaceInfo();
	}
	void FTFace::_updateFaceInfo() {
		auto& met = _face->size->metrics;
		_faceInfo.baseline = (_face->height + _face->descender) *
							met.y_ppem / _face->units_per_EM;
		_faceInfo.height = met.height / FTUnit;
		_faceInfo.maxWidth = met.max_advance / FTUnit;
	}
	const char* FTFace::getFamilyName() const {
		return _face->family_name;
	}
	const char* FTFace::getStyleName() const {
		return _face->style_name;
	}
	size_t FTFace::getNFace() const {
		return _face->num_faces;
	}
	int FTFace::getFaceIndex() const {
		return _face->face_index;
	}
}
