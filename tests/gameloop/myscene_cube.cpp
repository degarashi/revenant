#include "myscene.hpp"
#include "cube.hpp"
#include "../../camera3d.hpp"
#include "../../systeminfo.hpp"
#include "../../input.hpp"
#include "../../imgui_sdl2.hpp"
#include "../../sys_uniform.hpp"
#include "../../glx_if.hpp"

namespace rev::test {
	void MyScene::St_Cube::onExit(MyScene& self, ObjTypeId_OP) {
		auto dg = self.getDrawGroup();
		dg->remObj(_cube);
	}
	void MyScene::St_Cube::onEnter(MyScene& self, ObjTypeId_OP) {
		{
			const auto scr = mgr_info.getScreenSize();
			_camera = Camera3D::NewS();
			auto& c = *_camera;
			c.setAspect(scr.width / scr.height);
			c.setFov(frea::DegF(90));
			c.setZPlane(1e-2f, 1e3f);
		}
		{
			const auto hKb = Keyboard::OpenKeyboard();
			const auto hM = Mouse::OpenMouse(0);
			_act[Act::MoveX] = mgr_input.makeAction("move_x");
			_act[Act::MoveX]->linkButtonAsAxis(hKb, VKey::A, VKey::D);
			_act[Act::MoveY] = mgr_input.makeAction("move_y");
			_act[Act::MoveY]->linkButtonAsAxis(hKb, VKey::Lshift, VKey::Space);
			_act[Act::MoveZ] = mgr_input.makeAction("move_z");
			_act[Act::MoveZ]->linkButtonAsAxis(hKb, VKey::S, VKey::W);
			_act[Act::DirX] = mgr_input.makeAction("dir_x");
			_act[Act::DirX]->addLink(hM, InputFlag::Axis, 0);
			_act[Act::DirY] = mgr_input.makeAction("dir_y");
			_act[Act::DirY]->addLink(hM, InputFlag::Axis, 1);
			_act[Act::DirBtn] = mgr_input.makeAction("dir_b");
			_act[Act::DirBtn]->addLink(hM, InputFlag::Button, 0);
		}
		{
			auto dg = self.getDrawGroup();
			_cube = CubeObj::NewS(Vec3{2});
			dg->addObj(_cube);
		}
		_fp.setSpeed(.5f);
		_fp.setDirSpeed(100.f);
		_fp.setOffset(Vec3(0.5f,0.5f,-3));
	}
	void MyScene::St_Cube::onUpdate(MyScene& self) {
		self._checkQuit();
		self._checkPause();
		self._checkSwitch();

		const auto hM = Mouse::OpenMouse(0);
		if(_act[Act::DirBtn]->isKeyPressed()) {
			if(!mgr_gui.pointerOnGUI()) {
				if(!_press) {
					hM->setMouseMode(MouseMode::Relative);
					_press = true;
				}
			}
		}
		if(_press) {
			_fp.update(
				_act[Act::MoveX], _act[Act::MoveY], _act[Act::MoveZ],
				_act[Act::DirX], _act[Act::DirY]
			);
			if(!_act[Act::DirBtn]->isKeyPressing()) {
				if(_press) {
					hM->setMouseMode(MouseMode::Absolute);
					_press = false;
				}
			}
		}
	}
	void MyScene::St_Cube::onDraw(const MyScene& self, IEffect& e) const {
		self._clearBg(e);
		self._showFPS(e);

		auto& e3 = e.ref3D();
		e3.setCamera(_camera);
		_camera->setPose(_fp);
	}
}