#include "resmgr_app.hpp"
#include "sprite_scene.hpp"
#include "bsprite.hpp"
#include "../../u_matrix2d.hpp"
#include "../../glx_if.hpp"
#include "../../camera2d.hpp"
#include "../../systeminfo.hpp"
#include "../../tls_data.hpp"
#include "../../gl_resource.hpp"
#include "../../texture_filter.hpp"
#include "frea/src/random/vector.hpp"
#include <boost/format.hpp>

namespace rev::test {
	struct SpriteScene::St_Default : StateT<St_Default, St_Base> {
		void onEnter(SpriteScene& self, const ObjTypeId_OP id) override {
			St_Base::onEnter(self, id);

			self._camera = Camera2D::NewS();
			const auto scr = mgr_info.getScreenSize();
			self._camera->setAspectRatio(scr.width / scr.height);
			self._camera->refPose().identity();

			auto& mt = tls_mt.get();
			auto rfi = mt.getUniformF<int>();
			auto rf = mt.getUniformF<float>();
			HTex tex[6];
			auto filter = mgr_gl.createTexFilter();
			filter->setFilter(true, true);
			for(uint32_t i=0 ; i<countof(tex) ; i++) {
				auto src = mgr_gl.loadTexture(UserURI((boost::format("spr%1%.png") % i).str()), MipState::MipmapLinear);
				tex[i] = mgr_gl.attachTexFilter(src, filter);
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
				self._sprite[i] = obj;
			}
		}
		void onExit(SpriteScene& self, const ObjTypeId_OP id) override {
			for(std::size_t i=0 ; i<NSprite ; i++) {
				self._sprite[i]->destroy();
			}
			St_Base::onExit(self, id);
		}
		void onDraw(const SpriteScene& self, IEffect& e) const override {
			St_Base::onDraw(self, e);

			auto& e2 = dynamic_cast<U_Matrix2D&>(e);
			e2.setCamera(self._camera);
		}
	};

	SpriteScene::SpriteScene() {
		setStateNew<St_Default>();
		_makeGui();
	}
}
