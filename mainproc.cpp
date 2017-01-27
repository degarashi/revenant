#include "mainproc.hpp"
#include "scene.hpp"
#include "output.hpp"
#include "sharedata.hpp"
#include "glx_if.hpp"

namespace rev {
	// ---------------- MainProc ----------------
	const bool detail::c_pauseDefault = false;
	bool MainProc::runU() {
		if(mgr_scene.onUpdate()) {
			auto lk = g_system_shared.lockC();
			if(auto fx = lk->fx.lock()) {
				lk.unlock();
				fx->beginTask();
				mgr_scene.onDraw(*fx);
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
