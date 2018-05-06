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
#include "drawtoken/scissor.hpp"
#include "unituple/operator.hpp"
#include "tech_if.hpp"
#include "primitive.hpp"
#include "gl_state.hpp"

namespace rev {
	namespace {
		const HPrim c_invalidPrimitive = std::make_shared<Primitive>();
	}
	GLEffect::GLEffect():
		_primitive(std::make_shared<Primitive>()),
		_primitive_prev(std::make_shared<Primitive>()),
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

		_clean_drawvalue();
		_hFb = HFb();
		// ビューポートはデフォルトでフルスクリーンに初期化
		setViewport({false, {0,1,0,1}});
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
	HTech GLEffect::setTechnique(const HTech& tech) {
		if(tech == _tech_sp)
			return tech;
		_clean_drawvalue();

		const auto prev_tech = _tech_sp;
		_tech_sp = tech;
		_uniformEnt.setProgram(tech->getProgram());
		// 各種セッティングをするTokenをリストに追加
		_tech_sp->getProgram()->getDrawToken(_tokenML);
		// MEMO: Techにマルチスレッドでアクセスしてしまってるので要改善
		_tokenML.allocate<draw::UserFunc>([t = _tech_sp](){
			auto& sv = t->getSetting();
			for(auto& s : sv)
				s->apply();
		});
		// Uniformデフォルト値読み込み
		_uniformEnt.copyFrom(_tech_sp->getDefaultValue());
		return prev_tech;
	}
	const HTech& GLEffect::getTechnique() const noexcept {
		return _tech_sp;
	}
	void GLEffect::_outputFramebuffer() {
		const auto set_viewrect = [this](){
			using T = draw::Viewport;
			const draw::Viewport vp(_viewrect);
			new(_tokenML.allocate_memory(sizeof(T), draw::CalcTokenOffset<T>())) T(vp);
			_bView = false;
		};
		const auto set_scissorrect = [this](){
			using T = draw::Scissor;
			const T sc(_scissorrect);
			new(_tokenML.allocate_memory(sizeof(T), draw::CalcTokenOffset<T>())) T(sc);
			_bScissor = false;
		};
		if(_hFb) {
			auto& fb = *_hFb;
			if(fb)
				fb->getDrawToken(_tokenML);
			else
				GLFBufferTmp(0, mgr_info.getScreenSize()).getDrawToken(_tokenML);
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
	void GLEffect::clearFramebuffer(const draw::ClearParam& param) {
		_outputFramebuffer();
		_tokenML.allocate<draw::Clear>(param);
		mgr_drawtask.refWriteEnt().append(std::move(_tokenML));
	}
	void GLEffect::draw() {
		applyUniform(_uniformEnt, *_tech_sp->getProgram());
		{
			auto& u = _uniformEnt;
			auto& res = u.getResult().token;
			res.iterateC([&dst = _tokenML](const auto* t){
				t->clone(dst);
			});
			u.clearValue();
		}
		_outputFramebuffer();
		_diffCount.buffer += _getDifference();
		// set V/IBuffer(VDecl)
		draw::Stream vs = _primitive->extractVertexData(_tech_sp->getVAttr());
		const auto& p = _primitive;
		if(!p->ib) {
			_tokenML.allocate<draw::Draw>(
				std::move(vs),
				p->drawMode,
				p->withoutIndex.first,
				p->withoutIndex.count
			);
			++_diffCount.drawNoIndexed;
		} else {
			const auto str = p->ib->getStride();
			const auto szF = GLIBuffer::GetSizeFlag(str);
			_tokenML.allocate<draw::DrawIndexed>(
				std::move(vs),
				p->drawMode,
				p->withIndex.count,
				szF,
				p->withIndex.offsetElem*str
			);
			++_diffCount.drawIndexed;
		}
		mgr_drawtask.refWriteEnt().append(std::move(_tokenML));
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
	// MEMO: Viewportのコードと重複しているので後でなんとかする
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
