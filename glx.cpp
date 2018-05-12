#include "glx.hpp"
#include "gl_framebuffer.hpp"
#include "systeminfo.hpp"
#include "gl_error.hpp"
#include "gl_program.hpp"
#include "unituple/operator.hpp"
#include "tech_if.hpp"
#include "primitive.hpp"
#include "gl_state.hpp"
#include "gl_buffer.hpp"

namespace rev {
	namespace {
		const FWPrim c_invalidPrimitive{Primitive()};
	}
	GLEffect::GLEffect():
		_primitive(c_invalidPrimitive),
		_primitive_prev(c_invalidPrimitive),
		_viewrect(false, {0,1,0,1}),
		_scissorrect(false, {0,1,0,1}),
		_bView(true),
		_bScissor(true)
	{}
	diff::Buffer GLEffect::_getDifference() {
		diff::Buffer diff = {};
		if(_primitive != _primitive_prev) {
			const auto d = _primitive->getDifference(*_primitive_prev);
			diff.vertex += d.first;
			diff.index += d.second;
			_primitive_prev = _primitive;
		}
		return diff;
	}
	void GLEffect::_reset() {
		_primitive = _primitive_prev = c_invalidPrimitive;
		_writeEnt = nullptr;

		_clean_drawvalue();
		_hFb = HFb();
		// ビューポートはデフォルトでフルスクリーンに初期化
		setViewport({false, {0,1,0,1}});
	}
	void GLEffect::_clean_drawvalue() {
		_tech_sp.reset();
		// セットされているUniform変数を未セット状態にする
		_cmdvec.clear();
		_uniformEnt.clearValue();
	}
	UniformEnt& GLEffect::refUniformEnt() noexcept {
		return _uniformEnt;
	}
	HTech GLEffect::setTechnique(const HTech& tech) {
		if(tech == _tech_sp)
			return tech;
		_clean_drawvalue();

		const auto prev_tech = _tech_sp;
		_tech_sp = tech;
		_uniformEnt.setProgram(tech->getProgram());
		// 各種セッティングをするTokenをリストに追加
		_tech_sp->getProgram()->dcmd_export(_cmdvec);
		auto& sv = _tech_sp->getSetting();
		for(auto& s : sv)
			s->dcmd_export(_cmdvec);
		// Uniformデフォルト値読み込み
		_uniformEnt.assign(_tech_sp->getDefaultValue());
		return prev_tech;
	}
	const HTech& GLEffect::getTechnique() const noexcept {
		return _tech_sp;
	}
	void GLEffect::_outputFramebuffer() {
		const auto set_viewrect = [this](){
			_cmdvec.add(DCmd_Viewport{_viewrect});
			_bView = false;
		};
		const auto set_scissorrect = [this](){
			_cmdvec.add(DCmd_Scissor{_scissorrect});
			_bScissor = false;
		};
		if(_hFb) {
			auto& fb = *_hFb;
			if(fb)
				fb->dcmd_export(_cmdvec);
			else
				GLFBufferTmp(0, mgr_info.getScreenSize()).dcmd_export(_cmdvec);
			_hFbPrev = fb;
			_hFb = spi::none;
			if(!_bView) {
				// スケールでViewportを指定していた場合、設定しなおす
				if(_viewrect.isRatio()) {
					set_viewrect();
				}
				// スケールでScissorを指定していた場合、設定しなおす
				if(_scissorrect.isRatio()) {
					set_scissorrect();
				}
			}
		}
		if(_bView) {
			set_viewrect();
		}
		if(_bScissor) {
			set_scissorrect();
		}
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
	void GLEffect::setPrimitive(const FWPrim& p) noexcept {
		_primitive = p;
	}
	void GLEffect::clearFramebuffer(const ClearParam& param) {
		_outputFramebuffer();
		_cmdvec.add(DCmd_Clear{
			.bColor = static_cast<bool>(param.color),
			.bDepth = static_cast<bool>(param.depth),
			.bStencil = static_cast<bool>(param.stencil),
			.color = param.color ? *param.color : frea::Vec4{},
			.depth = param.depth ? *param.depth : 0.f,
			.stencil = param.stencil ? *param.stencil : 0u
		});
		_writeEnt->append(_cmdvec);
		_cmdvec.clear();
	}
	void GLEffect::draw() {
		applyUniform(_uniformEnt, *_tech_sp->getProgram());
		_uniformEnt.dcmd_export(_cmdvec);
		_uniformEnt.clearValue();
		_outputFramebuffer();
		_diffCount.buffer += _getDifference();
		// set V/IBuffer(VDecl)
		_primitive->dcmd_export(_cmdvec, _tech_sp->getVAttr());
		{
			const auto& p = _primitive;
			if(!p->ib) {
				_cmdvec.add(DCmd_Draw{
						.mode = p->drawMode,
						.first = p->withoutIndex.first,
						.count = p->withoutIndex.count,
						});
				++_diffCount.drawNoIndexed;
			} else {
				const auto str = p->ib->getStride();
				const auto szF = GLIBuffer::GetSizeFlag(str);
				_cmdvec.add(DCmd_DrawIndexed{
						.mode = p->drawMode,
						.count = p->withIndex.count,
						.sizeF = szF,
						.offset = p->withIndex.offsetElem*str,
						});
				++_diffCount.drawIndexed;
			}
		}
		_writeEnt->append(_cmdvec);
		_cmdvec.clear();
	}
	void GLEffect::beginTask() {
		_reset();
		_writeEnt = &_task.beginTask();
		TupleZeroFill(_diffCount);
	}
	void GLEffect::endTask() {
		_task.endTask();
		_writeEnt = nullptr;
	}
	void GLEffect::execTask() {
		_task.execTask();
	}
	void GLEffect::clearTask() {
		_task.clear();
	}
	void GLEffect::setFramebuffer(const HFb& fb) {
		_hFb = fb;
	}
	HFb GLEffect::getFramebuffer() const {
		if(_hFb)
			return *_hFb;
		return _hFbPrev;
	}
	void GLEffect::resetFramebuffer() {
		setFramebuffer(HFb());
	}
	FBRect GLEffect::setViewport(const FBRect& r) {
		const auto prev = _viewrect;
		_viewrect = r;
		_bView = true;
		return prev;
	}
	const FBRect& GLEffect::getViewport() const noexcept {
		return _viewrect;
	}
	FBRect GLEffect::setScissor(const FBRect& r) {
		const auto prev = _viewrect;
		_scissorrect = r;
		_bScissor = true;
		return prev;
	}
	diff::Effect GLEffect::getDifference() const {
		return _diffCount;
	}
}
