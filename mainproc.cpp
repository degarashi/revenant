#include "mainproc.hpp"
#include "scene.hpp"
#include "output.hpp"
#include "sharedata.hpp"

namespace rev {
	// ---------------- MainProc ----------------
	const bool detail::c_pauseDefault = false;
	bool MainProc::runU() {
		if(mgr_scene.onUpdate()) {
			auto lk = g_system_shared.lock();
			if(auto fx = lk->fx.lock())
				mgr_scene.onDraw(*fx);
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
