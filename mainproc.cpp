#include "mainproc.hpp"
#include "scene.hpp"
#include "output.hpp"

namespace rev {
	// ---------------- DrawQuery ----------------
	DrawQuery::DrawQuery(const int us, const int maxSkip):
		_tp(Clock::now()),
		_usec(us),
		_skip(0),
		_maxSkip(maxSkip)
	{}
	bool DrawQuery::checkDraw() {
		const auto now = Clock::now();
		const auto dur = now - _tp;
		_tp = now;
		// 時間が残っていれば描画
		// 最大スキップフレームを超過してたら必ず描画
		const bool ret = (_skip >= _maxSkip || dur < Microseconds(_usec));
		if(!ret)
			++_skip;
		else
			_skip = 0;
		return ret;
	}

	// ---------------- MainProc ----------------
	const bool detail::c_pauseDefault = false;
	bool MainProc::runU() {
		return mgr_scene.onUpdate();
	}
	bool MainProc::onPause() {
		LogR(Verbose, "OnPause");
		return detail::c_pauseDefault;
	}
	void MainProc::onResume() {
		LogR(Verbose, "OnResume");
	}
	void MainProc::onStop() {
		LogR(Verbose, "OnStop");
	}
	void MainProc::onReStart() {
		LogR(Verbose, "OnRestart");
	}
}
