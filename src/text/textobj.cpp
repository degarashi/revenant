#include "../resmgr_app.hpp"
#include "text.hpp"
#include "vdecl.hpp"
#include "../gl/resource.hpp"
#include "../effect/primitive.hpp"
#include "../effect/drawtag.hpp"
#include "../effect/uniform/common.hpp"
#include "../effect/if.hpp"
#include "../effect/techmgr.hpp"

namespace rev::detail {
	TextObj::TextObj(Face &face, const FontId fontId, std::u32string &&s):
		_text(std::move(s)),
		_fontId(fontId),
		_faceName(face.getFaceName())
	{
		_cache.rectSize = {0,0};
		_prepareTextureAndVertex(face);
	}
	void TextObj::_prepareTextureAndVertex(Face &face) {
		D_Assert0(_cache.drawSet.empty() &&
				(_cache.rectSize == lubee::SizeF{0,0}));
		// CharPosリストの作成
		// 1文字につき4頂点 + インデックス6個
		struct CPair {
			const CharPos*	cp;
			const int ofsx, ofsy;
			const float timeval;

			CPair(const CharPos* c, const int x, const int y, const float t):
				cp(c),
				ofsx(x),
				ofsy(y),
				timeval(t)
			{}
		};
		// テクスチャが複数枚に渡る時はフォント頂点(座標)を使いまわし、UV-tだけを差し替え
		std::unordered_map<HTexSrcC, std::vector<CPair>>	tpM;
		{
			const auto& dp = face.getFaceRenderer(_fontId);
			const int height = dp.renderer.height();
			int ofsx = 0,
				ofsy = -height;
			const float dt = 1.f / _text.length();
			float t = 0;
			for(auto& c : _text) {
				auto* p = face.getCharPos(CharId(c, _fontId));
				// 幾つのテクスチャが要るのかカウントしつつ、フォントを配置
				if(c == U'\n') {
					ofsy -= height;
					ofsx = 0;
				} else {
					if(p->box.width() > 0)
						tpM[p->hTex].emplace_back(p, ofsx, ofsy, t);
					ofsx += p->space;
				}
				t += dt;
			}
		}

		const uint_fast16_t c_index[] = {0,1,2, 2,3,0};
		const std::pair<int,int> c_rectI[] = {{0,2}, {1,2}, {1,3}, {0,3}};

		const size_t nplane = tpM.size();
		_cache.drawSet.resize(nplane);
		_cache.rectSize = lubee::SizeF{0,0};
		auto itr = tpM.cbegin();
		for(size_t i=0 ; i<nplane ; i++, ++itr) {
			const std::vector<CPair>& cpl = itr->second;
			const size_t nC = cpl.size();
			size_t vbase = 0;

			// フォント配置
			ByteBuff vbuff(nC * 4 * sizeof(vertex::text));
			U16Buff ibuff(nC * 6);
			auto* pv = reinterpret_cast<vertex::text*>(&vbuff[0]);
			auto* pi = &ibuff[0];
			for(int i=0 ; i<nC ; i++) {
				auto& cp = cpl[i];
				// 頂点生成
				for(auto& r : c_rectI) {
					pv->pos = frea::Vec2(cp.ofsx + cp.cp->box.ar[r.first], cp.ofsy - cp.cp->box.ar[r.second]);
					pv->uvt = frea::Vec3(cp.cp->uv.ar[r.first], cp.cp->uv.ar[r.second], cp.timeval);
					_cache.rectSize.width = std::max(_cache.rectSize.width, pv->pos.x);
					_cache.rectSize.height = std::min(_cache.rectSize.height, pv->pos.y);
					++pv;
				}
				// インデックス生成
				for(auto idx : c_index)
					*pi++ = vbase + idx;
				vbase += 4;
			}

			auto& ds = _cache.drawSet[i];
			ds.hTex = mgr_gl.attachTexFilter(itr->first, s_filter.GetData());
			ds.nChar = nC;

			const HVb vb = mgr_gl.makeVBuffer(DrawType::Static);
			vb->initData(std::move(vbuff), sizeof(vertex::text));
			const HIb ib = mgr_gl.makeIBuffer(DrawType::Static);
			ib->initData(std::move(ibuff));
			ds.primitive = Primitive::MakeWithIndex(
				vertex::text::s_decl,
				DrawMode::Triangles,
				ib,
				ds.nChar*6,
				0,
				vb
			);
		}
	}
	void TextObj::exportDrawTag(DrawTag &d) const {
		if(_cache.drawSet.empty())
			return;
		constexpr size_t maxTex = sizeof(d.idTex)/sizeof(d.idTex[0]);
		size_t curTex = 0;
		auto& p = d.primitive;
		p = _cache.drawSet.front().primitive;
		for(auto& ds : _cache.drawSet) {
			if(curTex != maxTex)
				d.idTex[curTex++] = ds.hTex;
		}
	}
	void TextObj::onCacheLost(TextObjPrivate) {
		// フォントキャッシュを消去
		_cache.drawSet.clear();
		_cache.rectSize = {0,0};
	}
	void TextObj::onCacheReset(TextObjPrivate, Face &face) {
		// 再度テキストデータ(CharPosポインタ配列)を作成
		_prepareTextureAndVertex(face);
	}
	FontId& TextObj::refFontId(TextObjPrivate) {
		return _fontId;
	}
	const FontName_S& TextObj::getFaceName() const {
		return _faceName;
	}
	void TextObj::draw(IEffect &e) const {
		auto& c = dynamic_cast<U_Common&>(e);
		for(auto& ds : _cache.drawSet) {
			c.texture.diffuse = ds.hTex;
			e.setPrimitive(ds.primitive);
			e.draw();
		}
	}
	const lubee::SizeF& TextObj::getDrawSize() const {
		return _cache.rectSize;
	}
	const TextObj::DefaultTech TextObj::s_defaultTech;
	const TextObj::DefaultFilter TextObj::s_filter;
	namespace {
		const Name Tech_Id("Text|Default");
	}
	HTech TextObj::MakeData(DefaultTech*) {
		return mgr_tech.loadTechPass("text.glx")->getTechnique(Tech_Id);
	}
	HTech TextObj::GetDefaultTech() {
		return s_defaultTech.GetData();
	}
	HTexF TextObj::MakeData(DefaultFilter*) {
		return mgr_gl.createTexFilter();
	}
}
