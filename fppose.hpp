#pragma once
#include "beat/pose3d.hpp"
#include "drawable.hpp"
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
			void update(
				const HAct& move_x,
				const HAct& move_y,
				const HAct& move_z,
				const HAct& dir_x,
				const HAct& dir_y
			);
	};
}
