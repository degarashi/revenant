#pragma once
#include "../scenebase_obj.hpp"
#include "../../handle/camera.hpp"

namespace rev::test {
	class SpriteScene :
		public TestSceneObj<SpriteScene>
	{
		private:
			constexpr static std::size_t NSprite = 16;
			HCam2	_camera;
			HObj	_sprite[NSprite];

			struct St_Default;

		public:
			SpriteScene();
	};
}
