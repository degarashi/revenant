#pragma once
#include "scene.hpp"

namespace rev {
	class U_Scene : public Scene<U_Scene> {
		private:
			struct St_None;
		public:
			U_Scene();
	};
}
DEF_LUAIMPORT(rev::U_Scene)
