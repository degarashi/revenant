#include "myscene.hpp"
#include "bsprite.hpp"
#include "../../camera2d.hpp"
#include "../../systeminfo.hpp"
#include "../../sharedata.hpp"
#include "../../gl_resource.hpp"
#include "../../gl_texture.hpp"
#include "../../glx_if.hpp"
#include "../../sys_uniform.hpp"
#include "frea/random/vector.hpp"
#include <boost/format.hpp>

namespace rev::test {
	void MyScene::St_Sprite::onEnter(MyScene& self, ObjTypeId_OP) {
		_camera = Camera2D::NewS();
		const auto scr = mgr_info.getScreenSize();
		_camera->setAspectRatio(scr.width / scr.height);
		_camera->refPose().identity();

		auto lk = g_system_shared.lock();
		auto rfi = lk->mt.getUniformF<int>();
		auto rf = lk->mt.getUniformF<float>();
		HTex tex[6];
		for(uint32_t i=0 ; i<countof(tex) ; i++) {
			tex[i] = mgr_gl.loadTexture(UserURI((boost::format("spr%1%.png") % i).str()), MipState::MipmapLinear);

			tex[i]->setFilter(true, true);
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
	void MyScene::St_Sprite::onExit(MyScene&, ObjTypeId_OP) {
		for(std::size_t i=0 ; i<NSprite ; i++) {
			_sprite[i]->destroy();
		}
	}
	void MyScene::St_Sprite::onUpdate(MyScene& self) {
		self._checkQuit();
		self._checkPause();
		self._checkSwitch();
	}
	void MyScene::St_Sprite::onDraw(const MyScene& self, IEffect& e) const {
		self._clearBg(e);
		self._showFPS(e);
		auto& e2 = e.ref2D();
		e2.setCamera(_camera);
	}
}