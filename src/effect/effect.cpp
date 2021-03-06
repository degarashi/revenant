#include "effect.hpp"
#include "tech_if.hpp"
#include "primitive.hpp"
#include "../gl/framebuffer_tmp.hpp"
#include "../systeminfo.hpp"
#include "../gl/error.hpp"
#include "../gl/program.hpp"
#include "../gl/state.hpp"
#include "../gl/buffer.hpp"
#include "../unituple/operator.hpp"

namespace rev {
	GLEffect::GLEffect():
		_viewrect(false, {0,1,0,1}),
		_scissorrect(false, {0,1,0,1}),
		_bView(true),
		_bScissor(true)
	{}
	void GLEffect::_reset() {
		_primitive = _primitive_prev = nullptr;
		_writeEnt = nullptr;

		_tech_sp.reset();
		_uniformEnt = spi::none;
		_hFb = HFb();
		// ビューポートはデフォルトでフルスクリーンに初期化
		setViewport({false, {0,1,0,1}});
	}
	UniformEnt& GLEffect::refUniformEnt() noexcept {
		return *_uniformEnt;
	}
	void GLEffect::setTechnique(const HTech& tech) {
		if(tech == _tech_sp) {
			// Uniformの初期値設定だけする
			tech->dcmd_uniform(*_writeEnt);
			return;
		}

		// 前のTechのステートを初期値に戻す
		if(_tech_sp)
			_tech_sp->dcmd_resetState(*_writeEnt);
		_tech_sp = tech;
		_uniformEnt = spi::construct(*tech->getProgram(), *_writeEnt);
		// [Program + GLSetting + UniformDefault]
		tech->dcmd_setup(*_writeEnt);
	}
	const HTech& GLEffect::getTechnique() const noexcept {
		return _tech_sp;
	}
	void GLEffect::_outputFramebuffer() {
		const auto set_viewrect = [this](){
			_writeEnt->add(DCmd_Viewport{_viewrect});
			_bView = false;
		};
		const auto set_scissorrect = [this](){
			_writeEnt->add(DCmd_Scissor{_scissorrect});
			_bScissor = false;
		};
		if(_hFb) {
			auto& fb = *_hFb;
			if(fb)
				fb->dcmd_export(*_writeEnt);
			else
				GLFBufferTmp(0, mgr_info.getScreenSize()).dcmd_export(*_writeEnt);
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
	void GLEffect::setPrimitive(const HPrim& p) noexcept {
		_primitive = p;
	}
	void GLEffect::clearFramebuffer(const ClearParam& param) {
		_outputFramebuffer();
		_writeEnt->add(DCmd_Clear{
			.bColor = static_cast<bool>(param.color),
			.bDepth = static_cast<bool>(param.depth),
			.bStencil = static_cast<bool>(param.stencil),
			.color = param.color ? *param.color : frea::Vec4{},
			.depth = param.depth ? *param.depth : 0.f,
			.stencil = param.stencil ? *param.stencil : 0u
		});
	}
	void GLEffect::draw() {
		applyUniform(*_uniformEnt, *_tech_sp->getProgram());
		_outputFramebuffer();

		// set V/IBuffer(VDecl)
		const auto vmap = _tech_sp->getVMap();
		if(_primitive_prev) {
			_primitive->dcmd_export_diff(*_writeEnt, *_primitive_prev, vmap);
		} else {
			_primitive->dcmd_export(*_writeEnt, vmap);
		}
		_primitive_prev = _primitive;
	}
	void GLEffect::beginTask() {
		_reset();
		_writeEnt = &_task.beginTask();
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
	draw::IQueue& GLEffect::refQueue() const {
		D_Assert0(_writeEnt);
		return *_writeEnt;
	}
}
