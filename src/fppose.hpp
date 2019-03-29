#pragma once
#include "beat/src/pose3d.hpp"
#include "object/drawable.hpp"
#include "handle/input.hpp"

namespace rev {
	class FPPose :
		public beat::g3::Pose,
		public lubee::AAllocator<FPPose>
	{
		private:
			using Vec2 = frea::Vec2;
			using Vec3 = frea::Vec3;
			using AQuat = frea::AQuat;
			using DegF = frea::DegF;
			float	_speed,
					_dir_speed;
			DegF	_yaw,
					_pitch,
					_roll;

		public:
			FPPose();
			void setSpeed(float s) noexcept;
			void setDirSpeed(float s) noexcept;
			DegF& yaw() noexcept;
			DegF& pitch() noexcept;
			DegF& roll() noexcept;
			void update(
				const HActC& move_x,
				const HActC& move_y,
				const HActC& move_z,
				const HActC& dir_x,
				const HActC& dir_y
			);
	};
}
