#include "myscene.hpp"
#include "bsprite.hpp"
#include "../../camera2d.hpp"
#include "../../systeminfo.hpp"
#include "../../gl_resource.hpp"
#include "../../gl_texture.hpp"
#include "../../glx_if.hpp"
#include "../../u_matrix2d.hpp"
#include "frea/random/vector.hpp"
#include "../../tls_data.hpp"
#include <boost/format.hpp>

namespace rev::test {
	void MyScene::St_Sprite::onEnter(MyScene& self, ObjTypeId_OP id) {
		St_Base::onEnter(self, id);

		_camera = Camera2D::NewS();
		const auto scr = mgr_info.getScreenSize();
		_camera->setAspectRatio(scr.width / scr.height);
		_camera->refPose().identity();

		auto& mt = tls_mt.get();
		auto rfi = mt.getUniformF<int>();
		auto rf = mt.getUniformF<float>();
		HTex tex[6];
		for(uint32_t i=0 ; i<countof(tex) ; i++) {
			tex[i] = mgr_gl.loadTexture(UserURI((boost::format("spr%1%.png") % i).str()), MipState::MipmapLinear);

			tex[i]->filter().setFilter(true, true);
		}
		using frea::random::GenVecUnit;
		using frea::random::GenVec;
		auto dg = self.getDrawGroup();
		auto& ug = self.getUpdGroupRef();
		for(std::size_t i=0 ; i<NSprite ; i++) {
			const auto vDir = GenVecUnit<Vec2>(rf);
			const auto vPos = GenVec<Vec2>(rf, {-1.f, 1.f});
			auto obj = std::make_shared<BoundingSprite>(dg, tex[rfi({0,5})], vPos, vDir*0.01f);
			obj->setScaling(Vec2{0.2,0.2});
			ug.addObj(obj);
			_sprite[i] = obj;
		}
	}
	void MyScene::St_Sprite::onExit(MyScene& self, ObjTypeId_OP id) {
		for(std::size_t i=0 ; i<NSprite ; i++) {
			_sprite[i]->destroy();
		}
		St_Base::onExit(self, id);
	}
	void MyScene::St_Sprite::onDraw(const MyScene& self, IEffect& e) const {
		St_Base::onDraw(self, e);

		auto& e2 = dynamic_cast<U_Matrix2D&>(e);
		e2.setCamera(_camera);
	}
}
