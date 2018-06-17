#ifdef DEBUGGUI_ENABLED
#include "mygui.hpp"
#include "../../imgui/imgui.h"
#include "../../debug_gui/window.hpp"
#include "../../debug_gui/mainmenubar.hpp"
#include "../../debug_gui/menu.hpp"
#include "../../debug_gui/print.hpp"
#include "resource_view.hpp"
#include "../../debug_gui/resource_window.hpp"
#include "../../drawgroup.hpp"
#include "../../gl_resource.hpp"
#include "../../gpu.hpp"
#include "../../debug_gui/profiler.hpp"

namespace rev {
	namespace test {
		struct MyGUI::St : StateT<St> {
			void onUpdate(MyGUI& self) override {
				self._logger.update();
			}
			void onDraw(const MyGUI& self, IEffect&) const override {
				ImGui::StyleColorsDark();
				if(const auto bar = debug::MainMenuBarPush()) {
					self._resview->drawMenu();
					if(const auto m = debug::MenuPush("Other")) {
						ImGui::MenuItem("GPU-Info", nullptr, &self._show.gpu);
						ImGui::MenuItem("Log", nullptr, &self._show.log);
						ImGui::MenuItem("Demo", nullptr, &self._show.demo);
						ImGui::MenuItem("Profiler", nullptr, &self._show.profile);
					}
				}
				if(self._show.demo)
					ImGui::ShowDemoWindow();
				if(self._show.log) {
					self._logger.drawGUI(&self._show.log);
				}
				if(self._show.gpu) {
					if(const auto w = debug::WindowPush("GPU-Info", &self._show.gpu, {640, 480})) {
						debug::Show("", self._gpu);
					}
				}
				if(self._show.profile) {
					if(const auto _ = debug::WindowPush("Profiler")) {
						self._profiler->draw();
					}
				}
				debug::ResourceWindow::Draw();
				self._resview->drawView();
			}
			void onConnected(MyGUI& self, const HGroup&) override {
				self._dg->addObj(self.shared_from_this());
			}
			void onDisconnected(MyGUI& self, const HGroup&) override {
				self._dg->remObj(self.shared_from_this());
			}
		};
		MyGUI::MyGUI(const HDGroup& dg):
			_dg(dg),
			_gpu(mgr_gl.makeResource<GPUInfo>()),
			_resview(std::make_shared<ResourceView>()),
			_profiler(std::make_shared<debug::Profiler>())
		{
			prof::g_param.lock()->setInterval(Seconds(0));
			setStateNew<St>();
		}
		const char* MyGUI::getDebugName() const noexcept {
			return "MyGUI";
		}
	}
}
#endif
