#include "font.hpp"
#include "lane.hpp"
#include "emplace.hpp"
#include "gl_resource.hpp"
#include "gl_texture.hpp"
#include "gl_buffer.hpp"
#include "drawtag.hpp"
#include <boost/lexical_cast.hpp>
#include "glx_if.hpp"
#include "vdecl.hpp"
#include "sys_uniform_value.hpp"

namespace rev {
	namespace {
		// 最低サイズ2bits, Layer1=4bits, Layer0=6bits = 12bits(4096)
		using LAlloc = LaneAlloc<6,4,2>;
	}
	// --------------------------- Face::DepPair ---------------------------
	Face::DepPair::DepPair(const String_SP& name, const lubee::PowSize& sfcSize, CCoreID cid):
		dep(*name, cid), cplane(sfcSize, dep.height(), LaneAlloc_UP(new LAlloc()))
	{}
	// --------------------------- Face ---------------------------
	// フォントのHeightとラインのHeightは違う！
	Face::Face(const String_SP& name, const lubee::PowSize& size, CCoreID cid, FontChMap& m):
		faceName(name),
		coreID(cid),
		sfcSize(size),
		fontMap(m)
	{}
	bool Face::operator != (const std::string& name) const {
		return !(this->operator == (name));
	}
	bool Face::operator ==(const std::string& name) const {
		return *faceName == name;
	}
	bool Face::operator != (CCoreID cid) const {
		return !(this->operator == (cid));
	}
	bool Face::operator == (CCoreID cid) const {
		return coreID == cid;
	}
	Face::DepPair& Face::getDepPair(CCoreID coreID) {
		return TryEmplace(depMap, coreID, faceName, sfcSize, coreID).first->second;
	}
	const CharPos* Face::getCharPos(CharID chID) {
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
			cp.hTex = mgr_gl.getEmptyTexture();
		} else {
			LaneRaw lraw;
			dp.cplane.rectAlloc(lraw, res.second.width());
			cp.hTex = lraw.hTex;

			// ビットデータをglTexSubImage2Dで書き込む
			auto* u = reinterpret_cast<Texture_Mem*>(lraw.hTex.get());
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

	// --------------------------- VDecl for text ---------------------------
	const VDecl_SP& DrawDecl<drawtag::text>::GetVDecl() {
		static VDecl_SP vd(new VDecl({
			{0, 0, GL_FLOAT, GL_FALSE, 2, (GLuint)VSem::POSITION},
			{0, 8, GL_FLOAT, GL_FALSE, 3, (GLuint)VSem::TEXCOORD0}
		}));
		return vd;
	}
	// --------------------------- TextObj ---------------------------
	TextObj::TextObj(Face& face, CCoreID coreID, std::u32string&& s): _text(std::move(s)), _coreID(coreID), _faceName(face.faceName) {
		_init(face);
	}
	void TextObj::_init(Face& face) {
		// CharPosリストの作成
		// 1文字につき4頂点 + インデックス6個
		struct CPair {
			const CharPos*	cp;
			const int ofsx, ofsy;
			const float timeval;

			CPair(const CharPos* c, int x, int y, float t):
				cp(c),
				ofsx(x),
				ofsy(y),
				timeval(t)
			{}
		};
		// テクスチャが複数枚に渡る時はフォント頂点(座標)を使いまわし、UV-tだけを差し替え
		std::unordered_map<HTex, std::vector<CPair>>	tpM;
		{
			const auto& dp = face.getDepPair(_coreID);
			const int height = dp.dep.height();
			int ofsx = 0,
				ofsy = -height;
			const float dt = 1.f / _text.length();
			float t = 0;
			for(auto& c : _text) {
				auto* p = face.getCharPos(CharID(c, _coreID));
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

		const uint16_t c_index[] = {0,1,2, 2,3,0};
		const std::pair<int,int> c_rectI[] = {{0,2}, {1,2}, {1,3}, {0,3}};

		const int nplane = tpM.size();
		_drawSet.resize(nplane);
		_rectSize = lubee::SizeF{0,0};
		auto itr = tpM.cbegin();
		for(int i=0 ; i<nplane ; i++, ++itr) {
			auto& ds = _drawSet[i];
			const std::vector<CPair>& cpl = itr->second;
			const int nC = cpl.size();
			int vbase = 0;

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
					_rectSize.width = std::max(_rectSize.width, pv->pos.x);
					_rectSize.height = std::min(_rectSize.height, pv->pos.y);
					++pv;
				}
				// インデックス生成
				for(auto idx : c_index)
					*pi++ = vbase + idx;
				vbase += 4;
			}

			ds.hTex = itr->first;
			ds.nChar = nC;
			// GLバッファにセット
			ds.hVb = mgr_gl.makeVBuffer(DrawType::Static);
			ds.hVb->initData(std::move(vbuff), sizeof(vertex::text));
			ds.hIb = mgr_gl.makeIBuffer(DrawType::Static);
			ds.hIb->initData(std::move(ibuff));
		}
	}
	void TextObj::exportDrawTag(DrawTag& d) const {
		constexpr int maxVb = sizeof(d.idVBuffer)/sizeof(d.idVBuffer[0]),
					maxTex = sizeof(d.idTex)/sizeof(d.idTex[0]);
		int curVb = 0,
			curTex = 0;
		bool bFirst = true;
		for(auto& ds : _drawSet) {
			if(curVb != maxVb)
				d.idVBuffer[curVb++] = ds.hVb;
			if(curTex != maxTex)
				d.idTex[curTex++] = ds.hTex;
			if(bFirst) {
				d.idIBuffer = ds.hIb;
				bFirst = false;
			}
		}
	}
	void TextObj::onCacheLost() {
		// フォントキャッシュを消去
		_drawSet.clear();
	}
	void TextObj::onCacheReset(Face& face) {
		// 再度テキストデータ(CharPosポインタ配列)を作成
		_init(face);
	}
	CCoreID& TextObj::refCoreID() {
		return _coreID;
	}
	const String_SP& TextObj::getFaceName() const {
		return _faceName;
	}
	void TextObj::draw(IEffect& e) const {
		e.setVDecl(DrawDecl<drawtag::text>::GetVDecl());
		for(auto& ds : _drawSet) {
			e.setUniform(unif::texture::Diffuse, ds.hTex);
			e.setVStream(ds.hVb, 0);
			e.setIStream(ds.hIb);
			e.drawIndexed(GL_TRIANGLES, ds.nChar*6, 0);
		}
	}
	const lubee::SizeF& TextObj::getSize() const {
		return _rectSize;
	}

	// --------------------------- FontGen ---------------------------
	namespace {
		String_SP ToSp(const std::string& s) { return String_SP(new FontName(s)); }
		String_SP ToSp(const String_SP& s) { return s; }
		const std::string& ToCp(const std::string& s) { return s; }
		const std::string& ToCp(const String_SP& s) { return *s; }
	}
	FontGen::FontGen(const lubee::PowSize& sfcSize): _sfcSize(sfcSize) {}

	template <class S>
	CCoreID FontGen::_makeCoreID(const S& name, CCoreID cid) {
		// cid = フォントファミリを無視した値
		auto itr = std::find(_faceL.begin(), _faceL.end(), ToCp(name));
		if(itr == _faceL.end()) {
			// 新しくFaceを作成
			cid.at<CCoreID::FaceID>() = static_cast<int>(_faceL.size());
			_faceL.emplace_back(ToSp(name), _sfcSize, cid, _fontMap);
		} else {
			cid.at<CCoreID::FaceID>() = itr - _faceL.begin();
		}
		return cid;
	}

	CCoreID FontGen::makeCoreID(const std::string& name, CCoreID cid) {
		return _makeCoreID(name, cid);
	}
	CCoreID FontGen::makeCoreID(const String_SP& name, CCoreID cid) {
		return _makeCoreID(name, cid);
	}
	Face& FontGen::_getArray(CCoreID cid) {
		// FaceList線形探索
		auto itr = std::find_if(_faceL.begin(), _faceL.end(), [cid](const Face& fc){
			return fc.coreID.at<CCoreID::FaceID>() == cid.at<CCoreID::FaceID>();
		});
		D_Assert0(itr != _faceL.end());
		return *itr;
	}
	void FontGen::clearCache(const bool bRestore) {
		// あくまでもキャッシュのクリアなので文字列データ等は削除しない
		_fontMap.clear();
		_faceL.clear();
		// 文字列クラスのキャッシュも破棄
		for(auto text : *this)
			text->onCacheLost();

		if(bRestore) {
			for(auto text: *this) {
				// FaceIDは使わず再度Face参照する
				auto& c = text->refCoreID();
				// TextからSPの名前を取り出してFaceIDを更新
				c = makeCoreID(text->getFaceName(), c);
				text->onCacheReset(_getArray(c));
			}
		}
	}
	std::u32string FontGen::_MakeTextTag(const CCoreID cid, const std::u32string& s) {
		// ハンドルキー = CCoreIDの64bit数値 + _ + 文字列
		std::basic_stringstream<char32_t>	ss;
		ss << boost::lexical_cast<std::u32string>(cid.value()) << U'_' << s;
		return ss.str();
	}
	HText FontGen::createText(const CCoreID cid, To32Str str) {
		std::u32string str32(str.moveTo());
		// CCoreIDを付加した文字列をキーにする
		auto& ar = _getArray(cid);
		const auto tag = _MakeTextTag(cid, str32);
		return
			acquireWithMake<TextObj>(
				tag,
				[&](const auto&, auto& ctor){
					ctor(ar, cid, std::move(str32));
				}
			).first;
	}
}
