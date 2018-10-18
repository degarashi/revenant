#include "litrot.hpp"
#include "../../input.hpp"
#include "../../imgui_sdl2.hpp"

namespace rev::test {
	// -------------- LitRot --------------
	LitRot::LitRot():
		_rot(frea::Quat::Identity())
	{}
	void LitRot::update(
		const HActC& dir_x,
		const HActC& dir_y,
		const frea::Vec3& right,
		const frea::Vec3& up
	) {
		const float dx = dir_x->getValueAsFloat()*70,
		dy = dir_y->getValueAsFloat()*70;
		const auto q = frea::Quat::Rotation(up, frea::DegF(-dx))
						* frea::Quat::Rotation(right, frea::DegF(-dy));
		_rot= q * _rot;
	}
	frea::Vec3 LitRot::getDir() const {
		return _rot.getDir();
	}

	// -------------- LitRot --------------
	LitRotObj::LitRotObj():
		_press(false)
	{
		const auto hM = Mouse::OpenMouse(0);
		_act[Act::DirX] = mgr_input.makeAction("lit_x");
		_act[Act::DirX]->addLink(hM, InputFlag::Axis, 0);
		_act[Act::DirY] = mgr_input.makeAction("lit_y");
		_act[Act::DirY]->addLink(hM, InputFlag::Axis, 1);
		_act[Act::LitBtn] = mgr_input.makeAction("lit_b");
		_act[Act::LitBtn]->addLink(hM, InputFlag::Button, 1);
	}
	void LitRotObj::update(
		const frea::Vec3& right,
		const frea::Vec3& up
	) {
		if(_press) {
			LitRot::update(_act[Act::DirX], _act[Act::DirY], right, up);
			if(!_act[Act::LitBtn]->isKeyPressing()) {
				const auto hM = Mouse::OpenMouse(0);
				hM->setMouseMode(MouseMode::Absolute);
				_press = false;
			}
		} else {
			if(_act[Act::LitBtn]->isKeyPressed()) {
				if(!mgr_gui.pointerOnGUI()) {
					const auto hM = Mouse::OpenMouse(0);
					hM->setMouseMode(MouseMode::Relative);
					_press = true;
				}
			}
		}
	}
}
