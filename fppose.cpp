#include "fppose.hpp"
#include "input.hpp"

namespace rev {
	FPPose::FPPose():
		_speed(1),
		_dir_speed(1),
		_yaw(0),
		_pitch(0),
		_roll(0)
	{
		identity();
	}
	void FPPose::setSpeed(const float s) noexcept {
		_speed = s;
	}
	void FPPose::setDirSpeed(const float s) noexcept {
		_dir_speed = s;
	}
	void FPPose::update(
		const HAct& move_x,
		const HAct& move_y,
		const HAct& move_z,
		const HAct& dir_x,
		const HAct& dir_y
	) {
		moveSide3D(move_x->getValueAsFloat() * _speed);
		moveUp2D(move_y->getValueAsFloat() * _speed);
		moveFwd3D(move_z->getValueAsFloat() * _speed);

		const float dx = dir_x->getValueAsFloat() * _dir_speed,
					dy = -dir_y->getValueAsFloat() * _dir_speed;
		_yaw += DegF(dx);
		_yaw.single();
		_pitch.set(lubee::Saturate<float>(_pitch.get() + dy, -89, 89));
		setRotation(AQuat::RotationYPR(_yaw, _pitch, _roll));
	}
}
