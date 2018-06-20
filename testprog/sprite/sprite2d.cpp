#include "sprite2d.hpp"
#include "../../gl_resource.hpp"
#include "../../gl_buffer.hpp"
#include "lubee/meta/countof.hpp"
#include "../../glx_if.hpp"
#include "../../uniform_ent.hpp"
#include "../../vdecl.hpp"
#include "../../techmgr.hpp"

// ----------------------- Sprite -----------------------
std::shared_ptr<rev::Primitive> Sprite2D::MakeData(lubee::IConst<0>) {
	// 大きさ1の矩形を定義して後でスケーリング
	const vertex::sprite tmpV[] = {
		{{0,1}, {0,0}},
		{{1,1}, {1,0}},
		{{1,0}, {1,1}},
		{{0,0}, {0,1}}
	};
	rev::HVb vb = mgr_gl.makeVBuffer(rev::DrawType::Static);
	vb->initData(tmpV, countof(tmpV), sizeof(vertex::sprite));

	const GLushort idx[] = {0,1,2, 2,3,0};
	rev::HIb ib = mgr_gl.makeIBuffer(rev::DrawType::Static);
	ib->initData(idx, countof(idx));

	return rev::Primitive::MakeWithIndex(
		vertex::sprite::s_vdecl,
		rev::DrawMode::Triangles,
		ib,
		6,
		0,
		vb
	);
}
rev::HTech Sprite2D::MakeData(lubee::IConst<1>) {
	return mgr_tech.loadTechPass("sprite2d.glx")->getTechnique("Sprite|Default");
}

Sprite2D::Sprite2D(const rev::HTex& t, const float z) {
	beat::g2::Pose::identity();
	_hTex = t;
	_zOffset = z;
	_zRange = {0.f, 1.f};
	_alpha = 1.f;
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
#include "../../output.hpp"
#include "../../tech_pass.hpp"
#include "../../u_matrix2d.hpp"
#include "../../u_common.hpp"
void Sprite2D::draw(rev::IEffect& e) const {
	e.setTechnique(_tech);
	{
		auto& c = dynamic_cast<rev::U_Common&>(e);
		c.texture.diffuse = _hTex;
		c.alpha = _alpha;
	}
	dynamic_cast<rev::U_Matrix2D&>(e).setWorld(getToWorld().convert<3,3>());
	e.setPrimitive(_primitive);
	e.draw();
}
void Sprite2D::outputDrawTag(rev::DrawTag& d) const {
	d.idTex[0] = _hTex;
	d.primitive = _primitive;
	d.technique = _tech;
	d.zOffset = _zOffset;
}

// ---------------------- Sprite頂点宣言 ----------------------
namespace vertex {
	const rev::FWVDecl sprite::s_vdecl = {
		rev::VDecl({
			{0,0, GL_FLOAT, GL_FALSE, 3, {rev::VSemEnum::POSITION, 0}},
			{0,12, GL_FLOAT, GL_FALSE, 2, {rev::VSemEnum::TEXCOORD, 0}}
		})
	};
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
#ifdef DEBUGGUI_ENABLED
const char* Sprite2DObj::getDebugName() const noexcept {
	return "Sprite2DObj";
}
#endif