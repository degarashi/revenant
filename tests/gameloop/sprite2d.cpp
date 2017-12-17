#include "sprite2d.hpp"
#include "../../gl_resource.hpp"
#include "../../gl_buffer.hpp"
#include "lubee/meta/countof.hpp"
#include "../../util/sys_unif.hpp"
#include "../../vdecl.hpp"

using GlxId = rev::IEffect::GlxId;
const rev::IdValue Sprite2D::T_Sprite2D = GlxId::GenTechId("Sprite", "Default");
// ----------------------- Sprite -----------------------
rev::WVb Sprite2D::s_wVb;
rev::WIb Sprite2D::s_wIb;
std::pair<rev::HVb, rev::HIb> Sprite2D::InitBuffer() {
	std::pair<rev::HVb, rev::HIb> ret;
	if(!(ret.first = s_wVb.lock())) {
		// 大きさ1の矩形を定義して後でスケーリング
		const vertex::sprite tmpV[] = {
			{{0,1}, {0,0}},
			{{1,1}, {1,0}},
			{{1,0}, {1,1}},
			{{0,0}, {0,1}}
		};
		ret.first = mgr_gl.makeVBuffer(rev::DrawType::Static);
		ret.first->initData(tmpV, countof(tmpV), sizeof(vertex::sprite));

		const GLushort idx[] = {0,1,2, 2,3,0};
		ret.second = mgr_gl.makeIBuffer(rev::DrawType::Static);
		ret.second->initData(idx, countof(idx));

		s_wVb = ret.first;
		s_wIb = ret.second;
	} else
		ret.second = s_wIb.lock();
	return ret;
}
Sprite2D::Sprite2D(const rev::HTex& t, const float z) {
	beat::g2::Pose::identity();
	_hTex = t;
	_zOffset = z;
	_zRange = {0.f, 1.f};
	_alpha = 1.f;
	std::tie(_hVb, _hIb) = InitBuffer();
}
void Sprite2D::setZOffset(const float z) {
	_zOffset = z;
}
void Sprite2D::setAlpha(const float a) {
	_alpha = a;
}
void Sprite2D::setZRange(const lubee::RangeF& r) {
	_zRange = r;
}
#include "../../sys_uniform_value.hpp"
#include "../../output.hpp"
void Sprite2D::draw(rev::IEffect& e) const {
	e.setTechPassId(T_Sprite2D);
	e.setVDecl(rev::DrawDecl<vdecl::sprite>::GetVDecl());
	e.setUniform(rev::unif2d::texture::Diffuse, _hTex);
	e.setUniform(rev::unif::Alpha, _alpha);
	e.ref2D().setWorld(getToWorld().convert<3,3>());
	e.setVStream(_hVb, 0);
	e.setIStream(_hIb);
	e.drawIndexed(GL_TRIANGLES, 6);
}
void Sprite2D::outputDrawTag(rev::DrawTag& d) const {
	d.idTex[0] = _hTex;
	d.idVBuffer[0] = _hVb;
	d.idIBuffer = _hIb;
	d.zOffset = _zOffset;
}

// ---------------------- Sprite頂点宣言 ----------------------
const rev::VDecl_SP& rev::DrawDecl<vdecl::sprite>::GetVDecl() {
	static rev::VDecl_SP vd(new rev::VDecl{
		{0,0, GL_FLOAT, GL_FALSE, 3, {rev::VSem::POSITION, 0}},
		{0,12, GL_FLOAT, GL_FALSE, 2, {rev::VSem::TEXCOORD, 0}}
	});
	return vd;
}

// ----------------------- Sprite2DObj -----------------------
struct Sprite2DObj::St : StateT<St> {
	void onUpdate(Sprite2DObj& self) override {
		self.outputDrawTag(self._dtag);
	}
	void onDraw(const Sprite2DObj& self, rev::IEffect& e) const override {
		self.draw(e);
	}
};
Sprite2DObj::Sprite2DObj(const rev::HTex& t, const float depth):
	Sprite2D(t, depth)
{
	setStateNew<St>();
}
