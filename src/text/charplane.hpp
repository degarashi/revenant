#pragma once
#include "lane_if.hpp"
#include "lubee/src/pow_value.hpp"

namespace rev {
	using LaneAlloc_UP = std::unique_ptr<ILaneAlloc>;
	//! CharPlaneと、その位置
	struct CharPos {
		HTexSrcC		hTex;		//!< フォントが格納されているテクスチャ (ハンドル所有権は別途CharPlaneが持つ)
		lubee::RectF	uv;			//!< 参照すべきUV値
		lubee::RectI	box;		//!< フォント原点に対する相対描画位置 (サイズ)
		size_t			space;		//!< カーソルを進めるべき距離
	};
	//! フォントのGLテクスチャ
	/*! 縦幅は固定。横は必要に応じて確保 */
	class CharPlane {
		private:
			lubee::PowSize	_sfcSize;
			const size_t	_fontH;		//!< フォント縦幅 (=height)
			LaneAlloc_UP	_lalloc;	//!< レーンの残り幅管理
			size_t			_nUsed;		//!< 割り当て済みのChar数(動作には影響しない)
			size_t			_nH;		//!< Plane一枚のLane数
			float			_dV;		//!< 1文字のVサイズ

			//! キャッシュテクスチャを一枚追加 -> Lane登録
			void _addCacheTex();

		public:
			//! フォントキャッシュテクスチャの確保
			/*! \param size[in] テクスチャ1辺のサイズ
				\param fh[in] Char高 */
			CharPlane(const lubee::PowSize& size, size_t fh, LaneAlloc_UP a);
			CharPlane(CharPlane&& cp) = default;
			//! 新しいChar登録領域を確保
			/*! まだどこにも登録されてないcodeである事はFontArray_Depが保証する
				\param[out] dst uv, hTexを書き込む */
			void rectAlloc(LaneRaw& dst, size_t width);
			const lubee::PowSize& getSurfaceSize() const;
	};
}
