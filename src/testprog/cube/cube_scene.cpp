#include "cube_scene.hpp"
#include "cube.hpp"
#include "shared.hpp"
#include "../../effect/uniform/matrix3d.hpp"
#include "../../effect/if.hpp"

namespace rev::test {
	struct CubeScene::St_Default : StateT<CubeScene, St_Base> {
		void onEnter(CubeScene& self, const ObjTypeId_OP id) override {
			St_Base::onEnter(self, id);

			auto dg = self.getDrawGroup();
			self._cube = CubeObj::NewS(Vec3{2});
			dg->addObj(self._cube);
		}
		void onUpdate(CubeScene& self) override {
			St_Base::onUpdate(self);
			auto& sh = *tls_shared;
			self._fpc.update(
				sh.act[UserShare::Act::CMoveX],
				sh.act[UserShare::Act::CMoveY],
				sh.act[UserShare::Act::CMoveZ],
				sh.act[UserShare::Act::CDirX],
				sh.act[UserShare::Act::CDirY],
				sh.act[UserShare::Act::CDirBtn]
			);
		}
		void onExit(CubeScene& self, const ObjTypeId_OP id) override {
			auto dg = self.getDrawGroup();
			dg->remObj(self._cube);
			St_Base::onExit(self, id);
		}
		void onDraw(const CubeScene& self, IEffect& e) const override {
			St_Base::onDraw(self, e);

			auto& e3 = dynamic_cast<U_Matrix3D&>(e);
			e3.setCamera(self._fpc.getCamera());
		}
	};
	CubeScene::CubeScene() {
		setStateNew<St_Default>();
		_makeGui();
	}
}
