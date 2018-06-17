#pragma once
#include "../handle/camera.hpp"
#include "../fppose.hpp"

namespace rev::test {
	class FPCamera {
		private:
			DefineEnum(Act,
				(MoveX)(MoveY)(MoveZ)
				(DirX)(DirY)(DirBtn)
			);
			HAct	_act[Act::_Num];
			HCam3	_camera;
			FPPose	_fp;
			bool	_press;

		public:
			FPCamera();
			void update();
			const HCam3& getCamera() const noexcept;
	};
}
