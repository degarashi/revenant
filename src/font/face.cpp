#include "../resmgr_app.hpp"
#include "font.hpp"
#include "lane.hpp"
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
	Face::DepPair::DepPair(const FontName_S& name, const lubee::PowSize& sfcSize, const FontId cid):
		dep(*name, cid), cplane(sfcSize, dep.height(), LaneAlloc_UP(new LAlloc()))
	{}
	// --------------------------- Face ---------------------------
	// フォントのHeightとラインのHeightは違う！
	Face::Face(const FontName_S& name, const lubee::PowSize& size, const FontId cid, FontChMap& m):
		faceName(name),
		fontId(cid),
		sfcSize(size),
		fontMap(m)
	{}
	bool Face::operator != (const std::string& name) const {
		return !(this->operator == (name));
	}
	bool Face::operator ==(const std::string& name) const {
		return *faceName == name;
	}
	bool Face::operator != (const FontId cid) const {
		return !(this->operator == (cid));
	}
	bool Face::operator == (const FontId cid) const {
		return fontId == cid;
	}
	Face::DepPair& Face::getDepPair(const FontId fontId) {
		return TryEmplace(depMap, fontId, faceName, sfcSize, fontId).first->second;
	}
	const CharPos* Face::getCharPos(const CharID chID) {
		// キャッシュが既にあればそれを使う
		auto itr = fontMap.find(chID);
		if(itr != fontMap.end())
			return &itr->second;

		// CharMapにエントリを作る
		CharPos& cp = fontMap[chID];
		// Dependクラスから文字のビットデータを取得
		auto& dp = getDepPair(chID);
		auto res = dp.dep.getChara(chID.code);
		// この時点では1ピクセル8bitなので、32bitRGBAに展開
		if(!res.first.empty())
			res.first = Convert8Bit_Packed32Bit(&res.first[0], res.second.width(), res.second.width(), res.second.height());
		cp.box = res.second;
		cp.space = dp.dep.width(chID.code);
		if(res.second.width() <= 0) {
			cp.uv *= 0;
			cp.hTex = mgr_gl.getEmptyTexture()->texture();
		} else {
			LaneRaw lraw;
			dp.cplane.rectAlloc(lraw, res.second.width());
			cp.hTex = lraw.hTex;

			// ビットデータをglTexSubImage2Dで書き込む
			auto* u = lraw.hTex.get();
			lraw.rect.x1 = lraw.rect.x0 + res.second.width();
			lraw.rect.y1 = lraw.rect.y0 + res.second.height();
			u->writeRect(AB_Byte(std::move(res.first)), lraw.rect, GL_UNSIGNED_BYTE);

			// UVオフセットを計算
			const auto& sz = dp.cplane.getSurfaceSize();
			float invW = 1.f / (static_cast<float>(sz.width)),
				invH = 1.f / (static_cast<float>(sz.height));

			float h = res.second.height();
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
