#pragma once
#include "../handle/camera.hpp"
#include "../fppose.hpp"

namespace rev::test {
	class FPCamera {
		private:
			HCam3	_camera;
			FPPose	_fp;
			bool	_press;

		public:
			FPCamera();
			void update(
				const HActC& move_x,
				const HActC& move_y,
				const HActC& move_z,
				const HActC& dir_x,
				const HActC& dir_y,
				const HActC& dir
			);
			const HCam3& getCamera() const noexcept;
	};
}
