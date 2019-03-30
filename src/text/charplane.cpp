#include "charplane.hpp"
#include "gl/resource.hpp"

namespace rev {
	CharPlane::CharPlane(const lubee::PowSize& size, const size_t fh, LaneAlloc_U a):
		_sfcSize(size),
		_fontH(fh),
		_lalloc(std::move(a)),
		_nUsed(0)
	{
		_dV = static_cast<decltype(_dV)>(fh) / _sfcSize.height;
		_nH = static_cast<decltype(_nH)>(1.f / _dV);
	}
	void CharPlane::_addCacheTex() {
		// OpenGLテクスチャ確保
		// MEMO: 環境によってはGL_RGBAが32bitでないかもしれないので対策が必要
		const auto hTex = mgr_gl.createTexture(_sfcSize, GL_RGBA, false, true);	// DeviceLost時: 内容のリストア有りで初期化
		// Lane登録
		lubee::RectI rect(0,_sfcSize.width, 0,_fontH);
		const auto nH = _nH;
		for(decltype(_nH) i=0 ; i<nH ; i++) {
			_lalloc->addFreeLane(hTex, rect);
			rect.addOffset(0, _fontH);
		}
	}
	void CharPlane::rectAlloc(LaneRaw& dst, const size_t width) {
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
}
