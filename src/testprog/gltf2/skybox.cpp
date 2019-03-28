#include "skybox.hpp"
#include "../../effect/if.hpp"
#include "../../camera3d.hpp"
#include "../../effect/primitive.hpp"
#include "../../effect/techmgr.hpp"
#include "../../effect/u_matrix3d.hpp"
#include "../../effect/u_common.hpp"
#include "../../effect/vdecl.hpp"
#include "../../gl/resource.hpp"
#include "../../gl/texture/filter.hpp"

namespace rev::test {
	namespace vertex {
		const FWVDecl skybox::s_vdecl = {
			VDecl({
				{0,0, GL_BYTE, GL_FALSE, 3, {VSemEnum::POSITION, 0}}
			})
		};
	}
	HPrim Skybox::MakeData(Prim*) {
		std::array<vertex::skybox, 8> tmpV;
		auto* dst = tmpV.data();
		for(int z=0 ; z<2 ; z++) {
			for(int y=0 ; y<2 ; y++) {
				for(int x=0 ; x<2 ; x++) {
					*dst++ = vertex::skybox{
						{int8_t(x*2-1), int8_t(y*2-1), int8_t(z*2-1)}
					};
				}
			}
		}
		D_Assert0(dst == tmpV.data()+tmpV.size());
		const auto vb = mgr_gl.makeVBuffer(DrawType::Static);
		vb->initData(tmpV.data(), tmpV.size(), sizeof(vertex::skybox));

		constexpr std::size_t NI = 6*6;
		std::array<uint8_t, NI> idx = {
			// Z-
			0,2,3, 3,1,0,
			// X-
			0,4,6, 6,2,0,
			// X+
			1,3,7, 7,5,1,
			// Y+
			2,6,7, 7,3,2,
			// Y-
			0,1,5, 5,4,0,
			// Z+
			7,6,4, 4,5,7,
		};
		const auto ib = mgr_gl.makeIBuffer(DrawType::Static);
		ib->initData(idx.data(), idx.size(), sizeof(uint8_t));

		return Primitive::MakeWithIndex(
			vertex::skybox::s_vdecl,
			DrawMode::Triangles,
			ib,
			NI,
			0,
			vb
		);
	}
	HTech Skybox::MakeData(Tech*) {
		return mgr_tech.loadTechPass("skybox.glx")->getTechnique("Skybox|Default");
	}
	HTex Skybox::MakeData(DefaultTex*) {
		auto f = mgr_gl.createTexFilter();
		f->setFilter(true, true);
		f->setWrap(WrapState::ClampToEdge);
		auto src = mgr_gl.loadCubeTexture(
			MipState::NoMipmap,
			spi::none,
			UserURI("morning/morning_lf.tga"),
			UserURI("morning/morning_rt.tga"),
			UserURI("morning/morning_up.tga"),
			UserURI("morning/morning_dn.tga"),
			UserURI("morning/morning_ft.tga"),
			UserURI("morning/morning_bk.tga")
		);
		return mgr_gl.attachTexFilter(src, f);
	}

	// --------------------- Skybox ---------------------
	void Skybox::setTexture(const HTexC& t) {
		_texture = t;
	}
	void Skybox::draw(IEffect& e) const {
		e.setTechnique(_tech);
		e.setPrimitive(_primitive);
		{
			auto& m3 = dynamic_cast<U_Matrix3D&>(e);
			frea::Vec3 ofs(0);
			if(const auto& c = m3.getCamera()) {
				ofs = c->getPose().getOffset();
			}
			constexpr float s = 1e2f;
			m3.setWorld(
				frea::AMat4::Scaling({s,s,s,1}) *
				frea::AMat4::Translation(ofs.convertI<4,3>(1))
			);
		}
		{
			auto& c = dynamic_cast<U_Common&>(e);
			c.texture.diffuse = _texture ? _texture : _defaultTex;
		}
		e.draw();
	}
}
