#pragma once
#include "../scenebase_obj.hpp"
#include "../fpcamera.hpp"

namespace rev::test {
	class CubeScene :
		public TestSceneObj<CubeScene>
	{
		private:
			FPCamera	_fpc;
			HDObj		_cube;
			struct St_Default;
		public:
			CubeScene();
	};
}
