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
	// -------------- GLEffect::Vertex --------------
	GLEffect::Vertex::Vertex() {}
	void GLEffect::Vertex::reset() {
		_spVDecl.reset();
		for(auto& v : _vbuff)
			v.reset();
	}
	void GLEffect::Vertex::setVDecl(const VDecl_SP& v) {
		_spVDecl = v;
	}
	void GLEffect::Vertex::setVBuffer(const HVb& hVb, const int n) {
		_vbuff[n] = hVb;
	}
	void GLEffect::Vertex::extractData(draw::VStream& dst,
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
	bool GLEffect::Vertex::operator != (const Vertex& v) const {
		if(_spVDecl != v._spVDecl)
			return true;
		for(int i=0 ; i<static_cast<int>(countof(_vbuff)) ; i++) {
			if(_vbuff[i] != v._vbuff[i])
				return true;
		}
		return false;
	}

	// -------------- GLEffect::Index --------------
	GLEffect::Index::Index() {}
	void GLEffect::Index::reset() {
		_ibuff.reset();
	}
	void GLEffect::Index::setIBuffer(const HIb& hIb) {
		_ibuff = hIb;
	}
	const HIb& GLEffect::Index::getIBuffer() const {
		return _ibuff;
	}
	void GLEffect::Index::extractData(draw::VStream& dst) const {
		if(_ibuff)
			dst.ibuff = draw::Buffer(_ibuff->getDrawToken());
	}
	bool GLEffect::Index::operator != (const Index& idx) const {
		return _ibuff != idx._ibuff;
	}

	// -------------- GLEffect --------------
	diff::Buffer GLEffect::_getDifference() {
		diff::Buffer diff = {};
		if(_vertex != _vertex_prev)
			++diff.vertex;
		if(_index != _index_prev)
			++diff.index;

		_vertex_prev = _vertex;
		_index_prev = _index;
		return diff;
	}
	void GLEffect::_reset() {
		_vertex.reset();
		_vertex_prev.reset();
		_index.reset();
		_index_prev.reset();

		_clean_drawvalue();
		_hFb = HFb();
		_viewport = spi::none;
	}
	void GLEffect::_clean_drawvalue() {
		_tech_sp.reset();
		// セットされているUniform変数を未セット状態にする
		clearUniformValue();
		_tokenML.clear();
	}
	void GLEffect::setTechnique(const Tech_SP& tech) {
		_clean_drawvalue();

		_tech_sp = tech;
		setProgram(tech->getProgram());
		// デフォルト値読み込み
		_refUniformValue().copyFrom(_tech_sp->getDefaultValue());
		// 各種セッティングをするTokenをリストに追加
		getProgram()->getDrawToken(_tokenML);
		_tokenML.allocate<draw::UserFunc>([tp_tmp = _tech_sp.get()](){
			tp_tmp->applySetting();
		});
	}
	void GLEffect::_outputFramebuffer() {
		if(_hFb) {
			auto& fb = *_hFb;
			if(fb)
				fb->getDrawToken(_tokenML);
			else
				GLFBufferTmp(0, mgr_info.getScreenSize()).getDrawToken(_tokenML);
			_hFbPrev = fb;
			_hFb = spi::none;

			// ビューポートはデフォルトでフルスクリーンに初期化
			if(!_viewport)
				_viewport = draw::Viewport(false, lubee::RectF{0,1,0,1});
		}
		if(_viewport) {
			using T = draw::Viewport;
			new(_tokenML.allocate_memory(sizeof(T), draw::CalcTokenOffset<T>())) T(*_viewport);
			_viewport = spi::none;
		}
	}
	void GLEffect::_outputDrawCall(draw::VStream& vs) {
		_outputFramebuffer();
		// set uniform value
		auto& u = _refUniformValue();
		if(!u.empty()) {
			// 中身shared_ptrなのでそのまま移動
			u.moveTo(_tokenML);
		}
		// set VBuffer(VDecl)
		_vertex.extractData(vs, _tech_sp->getVAttr());
		// set IBuffer
		_index.extractData(vs);
	}
	void GLEffect::_outputDrawCallIndexed(const GLenum mode, const GLsizei count, const GLenum sizeF, const GLuint offset) {
		draw::VStream vs;
		_outputDrawCall(vs);

		_tokenML.allocate<draw::DrawIndexed>(std::move(vs), mode, count, sizeF, offset);
	}
	void GLEffect::_outputDrawCall(const GLenum mode, const GLint first, const GLsizei count) {
		draw::VStream vs;
		_outputDrawCall(vs);

		_tokenML.allocate<draw::Draw>(std::move(vs), mode, first, count);
	}
	void GLEffect::onDeviceLost() {
		if(_bInit) {
			_bInit = false;
			_reset();
		}
	}
	void GLEffect::onDeviceReset() {
		if(!_bInit) {
			_bInit = true;
		}
	}
	void GLEffect::setVDecl(const VDecl_SP& decl) {
		_vertex.setVDecl(decl);
	}
	void GLEffect::setVStream(const HVb& vb, const int n) {
		_vertex.setVBuffer(vb, n);
	}
	void GLEffect::setIStream(const HIb& ib) {
		_index.setIBuffer(ib);
	}
	void GLEffect::clearFramebuffer(const draw::ClearParam& param) {
		_outputFramebuffer();
		_tokenML.allocate<draw::Clear>(param);
		mgr_drawtask.refWriteEnt().append(std::move(_tokenML));
	}
	void GLEffect::draw(const GLenum mode, const GLint first, const GLsizei count) {
		_prepareUniforms();
		_outputDrawCall(mode, first, count);
		mgr_drawtask.refWriteEnt().append(std::move(_tokenML));

		_diffCount.buffer += _getDifference();
		++_diffCount.drawNoIndexed;
	}
	void GLEffect::drawIndexed(const GLenum mode, const GLsizei count, const GLuint offsetElem) {
		_prepareUniforms();
		const HIb hIb = _index.getIBuffer();
		const auto str = hIb->getStride();
		const auto szF = GLIBuffer::GetSizeFlag(str);
		_outputDrawCallIndexed(mode, count, szF, offsetElem*str);
		mgr_drawtask.refWriteEnt().append(std::move(_tokenML));

		_diffCount.buffer += _getDifference();
		++_diffCount.drawIndexed;
	}
	void GLEffect::beginTask() {
		mgr_drawtask.beginTask(shared_from_this());
		_reset();
		TupleZeroFill(_diffCount);
	}
	void GLEffect::endTask() {
		mgr_drawtask.endTask();
	}
	void GLEffect::execTask() {
		mgr_drawtask.execTask();
	}
	void GLEffect::setFramebuffer(const HFb& fb) {
		_hFb = fb;
		_viewport = spi::none;
	}
	HFb GLEffect::getFramebuffer() const {
		if(_hFb)
			return *_hFb;
		return _hFbPrev;
	}
	void GLEffect::resetFramebuffer() {
		setFramebuffer(HFb());
	}
	void GLEffect::setViewport(const bool bPixel, const lubee::RectF& r) {
		_viewport = draw::Viewport(bPixel, r);
	}
	diff::Effect GLEffect::getDifference() const {
		return _diffCount;
	}
	void GLEffect::_prepareUniforms() {}
}
