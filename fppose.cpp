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
	frea::DegF& FPPose::yaw() noexcept {
		return _yaw;
	}
	frea::DegF& FPPose::pitch() noexcept {
		return _pitch;
	}
	frea::DegF& FPPose::roll() noexcept {
		return _roll;
	}
	void FPPose::setSpeed(const float s) noexcept {
		_speed = s;
	}
	void FPPose::setDirSpeed(const float s) noexcept {
		_dir_speed = s;
	}
	void FPPose::update(
		const HActC& move_x,
		const HActC& move_y,
		const HActC& move_z,
		const HActC& dir_x,
		const HActC& dir_y
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
