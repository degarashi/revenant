#include "u_scene.hpp"

namespace rev {
	struct U_Scene::St_None : StateT<St_None> {};
	U_Scene::U_Scene() {
		setStateNew<St_None>();
	}
}
