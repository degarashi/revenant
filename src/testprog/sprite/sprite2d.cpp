#include "sprite2d.hpp"
#include "../../effect/if.hpp"
#include "../../effect/techmgr.hpp"
#include "../../effect/uniform_ent.hpp"
#include "../../effect/vdecl.hpp"
#include "../../gl/buffer.hpp"
#include "../../gl/resource.hpp"
#include "lubee/src/meta/countof.hpp"

namespace rev::test {
	// ----------------------- Sprite -----------------------
	HPrim Sprite2D::MakeData(Prim*) {
		// 大きさ1の矩形を定義して後でスケーリング
		const vertex::sprite tmpV[] = {
			{{0,1}, {0,0}},
			{{1,1}, {1,0}},
			{{1,0}, {1,1}},
			{{0,0}, {0,1}}
		};
		HVb vb = mgr_gl.makeVBuffer(DrawType::Static);
		vb->initData(tmpV, countof(tmpV), sizeof(vertex::sprite));

		const GLushort idx[] = {0,1,2, 2,3,0};
		HIb ib = mgr_gl.makeIBuffer(DrawType::Static);
		ib->initData(idx, countof(idx));

		return Primitive::MakeWithIndex(
			vertex::sprite::s_vdecl,
			DrawMode::Triangles,
			ib,
			6,
			0,
			vb
		);
	}
	HTech Sprite2D::MakeData(Tech*) {
		return mgr_tech.loadTechPass("sprite2d.glx")->getTechnique("Sprite|Default");
	}
	Sprite2D::Sprite2D(const HTex& t, const float z) {
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
}

#include "../../output.hpp"
#include "../../effect/tech_pass.hpp"
#include "../../effect/u_matrix2d.hpp"
#include "../../effect/u_common.hpp"
namespace rev::test {
	void Sprite2D::draw(IEffect& e) const {
		e.setTechnique(_tech);
		{
			auto& c = dynamic_cast<U_Common&>(e);
			c.texture.diffuse = _hTex;
			c.alpha = _alpha;
		}
		dynamic_cast<U_Matrix2D&>(e).setWorld(getToWorld().convert<3,3>());
		e.setPrimitive(_primitive);
		e.draw();
	}
	void Sprite2D::outputDrawTag(DrawTag& d) const {
		d.idTex[0] = _hTex;
		d.primitive = _primitive;
		d.technique = _tech;
		d.zOffset = _zOffset;
	}

	// ---------------------- Sprite頂点宣言 ----------------------
	namespace vertex {
		const FWVDecl sprite::s_vdecl = {
			VDecl({
				{0,0, GL_FLOAT, GL_FALSE, 3, {VSemEnum::POSITION, 0}},
				{0,12, GL_FLOAT, GL_FALSE, 2, {VSemEnum::TEXCOORD, 0}}
			})
		};
	}

	// ----------------------- Sprite2DObj -----------------------
	struct Sprite2DObj::St : StateT<St> {
		void onUpdate(Sprite2DObj& self) override {
			self.outputDrawTag(self._dtag);
		}
		void onDraw(const Sprite2DObj& self, IEffect& e) const override {
			self.draw(e);
		}
	};
	Sprite2DObj::Sprite2DObj(const HTex& t, const float depth):
		Sprite2D(t, depth)
	{
		setStateNew<St>();
	}
	#ifdef DEBUGGUI_ENABLED
	const char* Sprite2DObj::getDebugName() const noexcept {
		return "Sprite2DObj";
	}
	#endif
}
