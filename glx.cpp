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
#include "primitive.hpp"
#include "gl_state.hpp"

namespace rev {
	GLEffect::GLEffect():
		_primitive(std::make_shared<Primitive>()),
		_primitive_prev(std::make_shared<Primitive>())
	{}
	diff::Buffer GLEffect::_getDifference() {
		diff::Buffer diff = {};
		if(_primitive != _primitive_prev) {
			const auto d = _primitive->getDifference(*_primitive_prev);
			diff.vertex += d.first;
			diff.index += d.second;
			_primitive = _primitive_prev;
		}
		return diff;
	}
	void GLEffect::_reset() {
		_primitive->reset();
		_primitive_prev->reset();

		_clean_drawvalue();
		_hFb = HFb();
		_viewport = spi::none;
	}
	void GLEffect::_clean_drawvalue() {
		_tech_sp.reset();
		// セットされているUniform変数を未セット状態にする
		_tokenML.clear();
		_uniformEnt.clearValue();
	}
	UniformEnt& GLEffect::refUniformEnt() noexcept {
		return _uniformEnt;
	}
	void GLEffect::setTechnique(const Tech_SP& tech) {
		_clean_drawvalue();

		_tech_sp = tech;
		_uniformEnt.setProgram(tech->getProgram());
		// 各種セッティングをするTokenをリストに追加
		_tech_sp->getProgram()->getDrawToken(_tokenML);
		_tokenML.allocate<draw::UserFunc>([tp_tmp = _tech_sp.get()](){
			auto& sv = tp_tmp->getSetting();
			for(auto& s : sv)
				s->apply();
		});
		// Uniformデフォルト値読み込み
		_uniformEnt.copyFrom(_tech_sp->getDefaultValue());
	}
	const Tech_SP& GLEffect::getTechnique() const noexcept {
		return _tech_sp;
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
		// set V/IBuffer(VDecl)
		_primitive->extractData(vs, _tech_sp->getVAttr());
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
	void GLEffect::setPrimitive(const Primitive_SP& p) noexcept {
		_primitive = p;
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
		const HIb hIb = _primitive->ib;
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
	void GLEffect::_prepareUniforms() {
		auto& res = _uniformEnt.getResult().token;
		res.iterateC([&dst = _tokenML](const auto* t){
			t->clone(dst);
		});
		_uniformEnt.clearValue();
	}
}
