#include "glx.hpp"
#include "gl_framebuffer.hpp"
#include "systeminfo.hpp"
#include "gl_error.hpp"
#include "gl_program.hpp"
#include "drawtoken/buffer.hpp"
#include "drawtoken/glx.hpp"
#include "drawtoken/clear.hpp"
#include "drawtoken/task.hpp"
#include "drawtoken/texture.hpp"
#include "unituple/operator.hpp"
#include "tech_if.hpp"

namespace rev {
	// -------------- GLEffect::Current::Vertex --------------
	GLEffect::Current::Vertex::Vertex() {}
	void GLEffect::Current::Vertex::reset() {
		_spVDecl.reset();
		for(auto& v : _vbuff)
			v.reset();
	}
	void GLEffect::Current::Vertex::setVDecl(const VDecl_SP& v) {
		_spVDecl = v;
	}
	void GLEffect::Current::Vertex::setVBuffer(const HVb& hVb, const int n) {
		_vbuff[n] = hVb;
	}
	void GLEffect::Current::Vertex::extractData(draw::VStream& dst,
												const VSemAttrV& vAttrId) const
	{
		Assert(_spVDecl, "VDecl is not set");
		dst.spVDecl = _spVDecl;
		for(int i=0 ; i<static_cast<int>(countof(_vbuff)) ; i++) {
			if(_vbuff[i])
				dst.vbuff[i] = _vbuff[i]->getDrawToken();
		}
		dst.vAttrId = vAttrId;
	}
	bool GLEffect::Current::Vertex::operator != (const Vertex& v) const {
		if(_spVDecl != v._spVDecl)
			return true;
		for(int i=0 ; i<static_cast<int>(countof(_vbuff)) ; i++) {
			if(_vbuff[i] != v._vbuff[i])
				return true;
		}
		return false;
	}

	// -------------- GLEffect::Current::Index --------------
	GLEffect::Current::Index::Index() {}
	void GLEffect::Current::Index::reset() {
		_ibuff.reset();
	}
	void GLEffect::Current::Index::setIBuffer(const HIb& hIb) {
		_ibuff = hIb;
	}
	const HIb& GLEffect::Current::Index::getIBuffer() const {
		return _ibuff;
	}
	void GLEffect::Current::Index::extractData(draw::VStream& dst) const {
		if(_ibuff)
			dst.ibuff = draw::Buffer(_ibuff->getDrawToken());
	}
	bool GLEffect::Current::Index::operator != (const Index& idx) const {
		return _ibuff != idx._ibuff;
	}

	// -------------- GLEffect::Current --------------
	diff::Buffer GLEffect::Current::getDifference() {
		diff::Buffer diff = {};
		if(vertex != vertex_prev)
			++diff.vertex;
		if(index != index_prev)
			++diff.index;

		vertex_prev = vertex;
		index_prev = index;
		return diff;
	}
	void GLEffect::Current::reset() {
		vertex.reset();
		vertex_prev.reset();
		index.reset();
		index_prev.reset();

		tech = spi::none;
		_clean_drawvalue();
		hFb = HFb();
		viewport = spi::none;
	}
	void GLEffect::Current::_clean_drawvalue() {
		pass = spi::none;
		tech_sp.reset();
		// セットされているUniform変数を未セット状態にする
		uniValue.clear();
	}
	void GLEffect::Current::setTech(const GLint idTech) {
		if(!tech || *tech != idTech) {
			// TechIdをセットしたらPassIdは無効になる
			tech = idTech;
			_clean_drawvalue();
		}
	}
	void GLEffect::Current::setPass(const GLint idPass, const TechPairV& tp) {
		// TechIdをセットせずにPassIdをセットするのは禁止
		if(!tech)
			throw GLE_Error( "tech is not selected");
		if(!pass || *pass != idPass) {
			_clean_drawvalue();
			pass = idPass;

			tech_sp = tp[*tech].pass[*pass].pass;
			// デフォルト値読み込み
			uniValue.copyFrom(tech_sp->getRuntime().defaultValue);
			// 各種セッティングをするTokenをリストに追加
			tech_sp->getProgram()->getDrawToken(tokenML);
			tokenML.allocate<draw::UserFunc>([&tp_tmp = *tech_sp](){
				tp_tmp.applySetting();
			});
		}
	}
	void GLEffect::Current::outputFramebuffer() {
		if(hFb) {
			auto& fb = *hFb;
			if(fb)
				fb->getDrawToken(tokenML);
			else
				GLFBufferTmp(0, mgr_info.getScreenSize()).getDrawToken(tokenML);
			hFbPrev = fb;
			hFb = spi::none;

			// ビューポートはデフォルトでフルスクリーンに初期化
			if(!viewport)
				viewport = draw::Viewport(false, lubee::RectF{0,1,0,1});
		}
		if(viewport) {
			using T = draw::Viewport;
			new(tokenML.allocate_memory(sizeof(T), draw::CalcTokenOffset<T>())) T(*viewport);
			viewport = spi::none;
		}
	}
	void GLEffect::Current::_outputDrawCall(draw::VStream& vs) {
		outputFramebuffer();
		// set uniform value
		if(!uniValue.empty()) {
			// 中身shared_ptrなのでそのまま移動
			uniValue.moveTo(tokenML);
		}
		// set VBuffer(VDecl)
		vertex.extractData(vs, tech_sp->getRuntime().vattr);
		// set IBuffer
		index.extractData(vs);
	}
	void GLEffect::Current::outputDrawCallIndexed(const GLenum mode, const GLsizei count, const GLenum sizeF, const GLuint offset) {
		draw::VStream vs;
		_outputDrawCall(vs);

		tokenML.allocate<draw::DrawIndexed>(std::move(vs), mode, count, sizeF, offset);
	}
	void GLEffect::Current::outputDrawCall(const GLenum mode, const GLint first, const GLsizei count) {
		draw::VStream vs;
		_outputDrawCall(vs);

		tokenML.allocate<draw::Draw>(std::move(vs), mode, first, count);
	}
	void GLEffect::onDeviceLost() {
		if(_bInit) {
			_bInit = false;
			for(auto& t : _tech) {
				for(auto& p : t.pass) {
					p.pass->ts_onDeviceLost();
				}
			}

			_current.reset();
		}
	}
	void GLEffect::onDeviceReset() {
		if(!_bInit) {
			_bInit = true;
			for(auto& t : _tech) {
				for(auto& p : t.pass) {
					p.pass->ts_onDeviceReset(*this);
				}
			}
		}
	}
	void GLEffect::setVDecl(const VDecl_SP& decl) {
		_current.vertex.setVDecl(decl);
	}
	void GLEffect::setVStream(const HVb& vb, const int n) {
		_current.vertex.setVBuffer(vb, n);
	}
	void GLEffect::setIStream(const HIb& ib) {
		_current.index.setIBuffer(ib);
	}
	void GLEffect::setTechnique(const int techId) {
		_current.setTech(techId);
		if(_unifId)
			_unifId->resultCur = nullptr;
	}
	void GLEffect::setPass(const int passId) {
		_current.setPass(passId, _tech);
		if(_unifId)
			_unifId->resultCur = &_unifId->result.at(_current.tech_sp.get());
	}
	GLint_OP GLEffect::_getPassId(const int techId, const Name& pass) const {
		const auto& tech = _tech[techId];
		const int nP = tech.pass.size();
		for(int i=0 ; i<nP ; i++) {
			if(tech.pass[i].name == pass)
				return i;
		}
		return spi::none;
	}
	GLint_OP GLEffect::getPassId(const Name& tech, const Name& pass) const {
		if(const auto idTech = getTechId(tech))
			return _getPassId(*idTech, pass);
		return spi::none;
	}
	GLint_OP GLEffect::getTechId(const Name& tech) const {
		const int nT = _tech.size();
		for(int i=0 ; i<nT ; i++) {
			if(_tech[i].name == tech)
				return i;
		}
		return spi::none;
	}
	GLint_OP GLEffect::getPassId(const Name& pass) const {
		if(!_current.tech)
			throw GLE_Error("tech is not selected");
		return _getPassId(*_current.tech, pass);
	}
	GLint_OP GLEffect::getCurPassId() const {
		return _current.pass;
	}
	GLint_OP GLEffect::getCurTechId() const {
		return _current.tech;
	}
	HProg GLEffect::getProgram(int techId, int passId) const {
		if(techId < 0) {
			if(!_current.tech)
				return HProg();
			techId = *_current.tech;
		}
		if(passId < 0) {
			if(!_current.pass)
				return HProg();
			passId = *_current.pass;
		}
		return _tech.at(techId).pass.at(passId).pass->getProgram();
	}
	draw::TokenBuffer& GLEffect::_makeUniformTokenBuffer(const GLint id) {
		return *_current.uniValue.makeTokenBuffer(id);
	}
	void GLEffect::clearFramebuffer(const draw::ClearParam& param) {
		_current.outputFramebuffer();
		_current.tokenML.allocate<draw::Clear>(param);
		mgr_drawtask.refWriteEnt().append(std::move(_current.tokenML));
	}
	void GLEffect::draw(const GLenum mode, const GLint first, const GLsizei count) {
		_prepareUniforms();
		_current.outputDrawCall(mode, first, count);
		mgr_drawtask.refWriteEnt().append(std::move(_current.tokenML));

		_diffCount.buffer += _current.getDifference();
		++_diffCount.drawNoIndexed;
	}
	void GLEffect::drawIndexed(const GLenum mode, const GLsizei count, const GLuint offsetElem) {
		_prepareUniforms();
		const HIb hIb = _current.index.getIBuffer();
		const auto str = hIb->getStride();
		const auto szF = GLIBuffer::GetSizeFlag(str);
		_current.outputDrawCallIndexed(mode, count, szF, offsetElem*str);
		mgr_drawtask.refWriteEnt().append(std::move(_current.tokenML));

		_diffCount.buffer += _current.getDifference();
		++_diffCount.drawIndexed;
	}
	// Uniform設定は一旦_unifMapに蓄積した後、出力
	void GLEffect::_makeUniformToken(draw::TokenDst& dst, const GLint id, const bool* b, const int n, const bool bT) const {
		int tmp[n];
		for(int i=0 ; i<n ; i++)
			tmp[i] = static_cast<int>(b[i]);
		_makeUniformToken(dst, id, static_cast<const int*>(tmp), 1, bT);
	}
	void GLEffect::_makeUniformToken(draw::TokenDst& dst, const GLint id, const int* iv, const int n, bool /*bT*/) const {
		MakeUniformToken<draw::Unif_Vec<int, 1>>(dst, id, id, iv, 1, n);
	}
	void GLEffect::_makeUniformToken(draw::TokenDst& dst, const GLint id, const float* fv, const int n, bool /*bT*/) const {
		MakeUniformToken<draw::Unif_Vec<float, 1>>(dst, id, id, fv, 1, n);
	}
	void GLEffect::_makeUniformToken(draw::TokenDst& dst, const GLint id, const double* dv, const int n, const bool bT) const {
		float tmp[n];
		for(int i=0 ; i<n ; i++)
			tmp[i] = static_cast<float>(dv[i]);
		_makeUniformToken(dst, id, static_cast<const float*>(tmp), n, bT);
	}
	void GLEffect::_makeUniformToken(draw::TokenDst& dst, const GLint id, const HTex* hTex, const int n, bool /*bT*/) const {
		// テクスチャユニット番号を検索
		const auto& tIdx = _current.tech_sp->getRuntime().texIndex;
		const auto itr = tIdx.find(id);
		Expect(itr != tIdx.end(), "texture index not found");
		if(itr != tIdx.end()) {
			const auto aId = itr->second;
			if(n > 1) {
				std::vector<const IGLTexture*> pTexA(n);
				for(int i=0 ; i<n ; i++)
					pTexA[i] = hTex[i].get();
				MakeUniformToken<draw::TextureA>(dst, id,
								id, hTex,
								pTexA.data(), aId, n);
				return;
			}
			D_Assert0(n==1);
			const HTex hTex2(*hTex);
			hTex2->getDrawToken(dst, id, 0, aId);
		}
	}
	GLint_OP GLEffect::getUniformId(const Name& name) const {
		D_Assert(_current.tech_sp, "Tech/Pass is not set");
		const auto& tech = *_current.tech_sp;
		const HProg hProg = tech.getProgram();
		D_Assert(hProg, "shader program handle is invalid");
		return hProg->getUniformId(name);
	}
	void GLEffect::beginTask() {
		mgr_drawtask.beginTask(shared_from_this());
		_current.reset();
		TupleZeroFill(_diffCount);
	}
	void GLEffect::endTask() {
		mgr_drawtask.endTask();
	}
	void GLEffect::execTask() {
		mgr_drawtask.execTask();
	}
	void GLEffect::_setConstantUniformList(const StrV* src) {
		_unifId = spi::construct();
		auto& r = _unifId->result;
		r.clear();

		_unifId->resultCur = nullptr;
		// 全てのTech&Passの組み合わせについてそれぞれUniform名を検索し、番号(GLint)を登録
		for(auto& t : _tech) {
			for(auto& p : t.pass) {
				auto* ptr = p.pass.get();
				auto& r2 = r[ptr];
				const GLProgram* prog = ptr->getProgram().get();
				for(auto& srcstr : *src) {
					if(const auto id = prog->getUniformId(srcstr)) {
						r2.push_back(*id);
					} else
						r2.push_back(-1);
				}
			}
		}
	}
	void GLEffect::_setConstantTechPassList(const StrPairV* src) {
		_techId = spi::construct();
		_techId->result.clear();

		// TechとPass名のペアについてそれぞれTechId, PassIdを格納
		auto& r = _techId->result;
		const int n = src->size();
		r.resize(n);
		for(int i=0 ; i<n ; i++) {
			auto& ip = r[i];
			ip.first = *getTechId((*src)[i].first);
			ip.second = *_getPassId(ip.first, (*src)[i].second);
		}
	}
	GLint_OP GLEffect::getUnifId(const IdValue id) const {
		// 定数値に対応するUniform変数が見つからない時は警告を出す
		if(static_cast<int>(_unifId->resultCur->size()) <= id._value)
			return spi::none;
		const auto ret = (*_unifId->resultCur)[id._value];
		if(ret < 0)
			return spi::none;
		return ret;
	}
	GLEffect::IdPair GLEffect::_getTechPassId(const IdValue id) const {
		Assert((static_cast<int>(_techId->result.size()) > id._value), "TechPass-ConstantId: Invalid Id (%d)", id._value);
		return _techId->result[id._value];
	}
	void GLEffect::setTechPassId(const IdValue id) {
		const auto ip = _getTechPassId(id);
		setTechnique(ip.first);
		setPass(ip.second);
	}
	void GLEffect::setFramebuffer(const HFb& fb) {
		_current.hFb = fb;
		_current.viewport = spi::none;
	}
	HFb GLEffect::getFramebuffer() const {
		if(_current.hFb)
			return *_current.hFb;
		return _current.hFbPrev;
	}
	void GLEffect::resetFramebuffer() {
		setFramebuffer(HFb());
	}
	void GLEffect::setViewport(const bool bPixel, const lubee::RectF& r) {
		_current.viewport = draw::Viewport(bPixel, r);
	}
	diff::Effect GLEffect::getDifference() const {
		return _diffCount;
	}
	void GLEffect::_prepareUniforms() {}
}
