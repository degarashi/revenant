#include "fpcamera.hpp"
#include "../systeminfo.hpp"
#include "../camera3d.hpp"
#include "../input.hpp"
#include "../imgui_sdl2.hpp"

namespace rev::test {
	FPCamera::FPCamera() {
		{
			const auto scr = mgr_info.getScreenSize();
			_camera = Camera3D::NewS();
			auto& c = *_camera;
			c.setAspect(scr.width / scr.height);
			c.setFov(frea::DegF(90));
			c.setZPlane(1e-2f, 1e3f);
		}
		_fp.setSpeed(.5f);
		_fp.setDirSpeed(100.f);
		_fp.setOffset(frea::Vec3(0.5f,0.5f,-3));
	}
	void FPCamera::update(
		const HActC& move_x,
		const HActC& move_y,
		const HActC& move_z,
		const HActC& dir_x,
		const HActC& dir_y,
		const HActC& dir
	) {
		const auto hM = Mouse::OpenMouse(0);
		if(dir->isKeyPressed()) {
			if(!mgr_gui.pointerOnGUI()) {
				if(!_press) {
					hM->setMouseMode(MouseMode::Relative);
					_press = true;
				}
			}
		}
		if(_press) {
			_fp.update(
				move_x, move_y, move_z,
				dir_x, dir_y
			);
			if(!dir->isKeyPressing()) {
				if(_press) {
					hM->setMouseMode(MouseMode::Absolute);
					_press = false;
				}
			}
		}
		_camera->setPose(_fp);
	}
	const HCam3& FPCamera::getCamera() const noexcept {
		return _camera;
	}
}
