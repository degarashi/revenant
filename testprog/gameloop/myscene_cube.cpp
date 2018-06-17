#include "myscene.hpp"
#include "cube.hpp"

namespace rev::test {
	void MyScene::St_Cube::onEnter(MyScene& self, ObjTypeId_OP id) {
		St_3D::onEnter(self, id);

		auto dg = self.getDrawGroup();
		_cube = CubeObj::NewS(Vec3{2});
		dg->addObj(_cube);
	}
	void MyScene::St_Cube::onExit(MyScene& self, ObjTypeId_OP id) {
		auto dg = self.getDrawGroup();
		dg->remObj(_cube);
		St_3D::onExit(self, id);
	}
}
