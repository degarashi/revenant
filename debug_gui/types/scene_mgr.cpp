#include "../../scene_mgr.hpp"
#include "../../scene_if.hpp"
#include "../id.hpp"
#include "../child.hpp"
#include "../column.hpp"
#include "../state_storage.hpp"
#include "../print.hpp"
#include "../../imgui/imgui.h"
#include "../listview.hpp"

namespace rev {
	bool SceneMgr::property(const bool) {
		if(const auto c = debug::ChildPush("SceneMgr", {0,0})) {
			debug::ListView(_scene.cbegin(), _scene.cend(),
					debug::ListViewFlag::NarrowColumn | debug::ListViewFlag::Filter);
		}
		return false;
	}
}
