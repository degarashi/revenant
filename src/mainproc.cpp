#include "mainproc.hpp"
#include "object/scene/mgr.hpp"
#include "output.hpp"
#include "sharedata.hpp"
#include "effect/if.hpp"
#include "imgui_sdl2.hpp"
#include "spine/src/profiler.hpp"

namespace rev {
	// ---------------- MainProc ----------------
	const bool detail::c_pauseDefault = false;
	bool MainProc::runU(const Duration delta) {
		bool cont;
		{
			SpiProfile(Scene_Update);
			cont = mgr_scene.onUpdate();
		}
		if(cont) {
			auto lk = g_system_shared.lockC();
			if(auto fx = lk->fx.lock()) {
				const auto w = lk->window.lock();
				lk.unlock();

				SpiProfile(Draw);
				fx->beginTask();
				mgr_gui.newFrame(fx, *w, delta);
				{
					SpiProfile(Scene_Draw);
					mgr_scene.onDraw(*fx);
				}
				mgr_gui.endFrame();
				fx->endTask();
			}
			return true;
		}
		return false;
	}
	bool MainProc::onPause() {
		LogR(Verbose, "OnPause");
		return mgr_scene.onPause();
	}
	void MainProc::onResume() {
		LogR(Verbose, "OnResume");
		mgr_scene.onResume();
	}
	void MainProc::onStop() {
		LogR(Verbose, "OnStop");
		mgr_scene.onStop();
	}
	void MainProc::onReStart() {
		LogR(Verbose, "OnRestart");
		mgr_scene.onReStart();
	}
}
