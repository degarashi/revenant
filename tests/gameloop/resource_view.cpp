#include "resource_view.hpp"
#include "../../imgui/imgui.h"
#include "../../camera2d.hpp"
#include "../../gl_resource.hpp"
#include "../../debug_gui/types/resmgr.hpp"
#include "../../debug_gui/types/resmgr_named.hpp"
#include "../../debug_gui/menu.hpp"
#include "../../debug_gui/window.hpp"
#include "../../scene_mgr.hpp"
#include "../../updgroup_if.hpp"

namespace rev {
	ResourceView::ResourceView():
		_show {}
	{}
	void ResourceView::drawMenu() {
		if(const auto m = debug::MenuPush("ResourceViewer")) {
			ImGui::MenuItem("Camera", nullptr, &_show.camera);
			ImGui::MenuItem("OpenGL", nullptr, &_show.gl);
			ImGui::MenuItem("Scene", nullptr, &_show.scene);
		}
	}
	void ResourceView::drawView() {
		if(_show.camera){
			if(const auto w = debug::WindowPush("Camera-Resource", &_show.camera, {640.f, 480.f})) {
				debug::Edit(nullptr, mgr_cam2d);
			}
		}
		if(_show.gl) {
			if(const auto w = debug::WindowPush("GL-Resource", &_show.gl, {640.f, 480.f})) {
				debug::Edit(nullptr, mgr_gl);
			}
		}
		if(_show.scene) {
			if(const auto w = debug::WindowPush("Scene", &_show.scene, {640.f, 480.f})) {
				mgr_scene.property(true);
			}
		}
	}
}
