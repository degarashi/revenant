#include "../resmgr_app.hpp"
#include "text.hpp"
#include "lanealloc.hpp"
#include "../emplace.hpp"
#include "../gl/resource.hpp"
#include "../gl/texture/mem.hpp"
#include "../gl/texture.hpp"

namespace rev::detail {
	namespace {
		// 最低サイズ2bits, Layer1=4bits, Layer0=6bits = 12bits(4096)
		using LAlloc = LaneAlloc<6,4,2>;
	}
	// --------------------------- Face::DepPair ---------------------------
	Face::DepPair::DepPair(const FontName_S& name, const lubee::PowSize& sfcSize, const FontId fid):
		renderer(*name, fid),
		cplane(sfcSize, renderer.height(), std::make_unique<LAlloc>())
	{}
	// --------------------------- Face ---------------------------
	// フォントのHeightとラインのHeightは違う！
	Face::Face(const FontName_S& name, const lubee::PowSize& size, const FontId fid, FontChMap& m):
		faceName(name),
		fontId(fid),
		sfcSize(size),
		fontMap(m)
	{}
	bool Face::operator != (const std::string &name) const {
		return !(this->operator == (name));
	}
	bool Face::operator ==(const std::string &name) const {
		return *faceName == name;
	}
	bool Face::operator != (const FontId fid) const {
		return !(this->operator == (fid));
	}
	bool Face::operator == (const FontId fid) const {
		return fontId == fid;
	}
	Face::DepPair& Face::getDepPair(const FontId fontId) {
		return TryEmplace(depMap, fontId, faceName, sfcSize, fontId).first->second;
	}
	const CharPos* Face::getCharPos(const CharId chID) {
		// キャッシュが既にあればそれを使う
		auto itr = fontMap.find(chID);
		if(itr != fontMap.end())
			return &itr->second;

		// CharMapにエントリを作る
		CharPos& cp = fontMap[chID];
		// Dependクラスから文字のビットデータを取得
		auto& dp = getDepPair(chID);
		auto res = dp.renderer.getChara(chID.code);
		// この時点では1ピクセル8bitなので、32bitRGBAに展開
		if(!res.pixel.empty())
			res.pixel = Convert8Bit_Packed32Bit(&res.pixel[0], res.rect.width(), res.rect.width(), res.rect.height());
		cp.box = res.rect;
		cp.space = dp.renderer.width(chID.code);
		if(res.rect.width() <= 0) {
			cp.uv *= 0;
			cp.hTex = mgr_gl.getEmptyTexture()->texture();
		} else {
			LaneRaw lraw;
			dp.cplane.rectAlloc(lraw, res.rect.width());
			cp.hTex = lraw.hTex;

			// ビットデータをglTexSubImage2Dで書き込む
			auto* u = lraw.hTex.get();
			lraw.rect.x1 = lraw.rect.x0 + res.rect.width();
			lraw.rect.y1 = lraw.rect.y0 + res.rect.height();
			u->writeRect(AB_Byte(std::move(res.pixel)), lraw.rect, GL_UNSIGNED_BYTE);

			// UVオフセットを計算
			const auto& sz = dp.cplane.getSurfaceSize();
			float invW = 1.f / (static_cast<float>(sz.width)),
				invH = 1.f / (static_cast<float>(sz.height));

			float h = res.rect.height();
			const auto& uvr = lraw.rect;
			cp.uv = lubee::RectF(
						uvr.x0 * invW,
						uvr.x1 * invW,
						uvr.y0 * invH,
						(uvr.y0+h) * invH
					);
		}
		return &cp;
	}
}
