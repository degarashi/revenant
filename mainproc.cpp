#include "mainproc.hpp"

namespace rev {
	const bool detail::c_pauseDefault = false;
	namespace {
		constexpr int MAX_SKIPFRAME = 3;
		constexpr int DRAW_THRESHOLD_USEC = 2000;
	}
	// ---------------- IMainProc::Query ----------------
	IMainProc::Query::Query(const Timepoint tp, const int skip):
		_bDraw(false),
		_tp(tp),
		_skip(skip)
	{}
	bool IMainProc::Query::canDraw() const {
		return true;
		const auto dur = Clock::now() - _tp;
		return _skip >= MAX_SKIPFRAME || dur <= Microseconds(DRAW_THRESHOLD_USEC);
	}
	void IMainProc::Query::setDraw(const bool bDraw) {
		_bDraw = bDraw;
	}
	bool IMainProc::Query::getDraw() const {
		return _bDraw;
	}

	// ---------------- IMainProc ----------------
	bool IMainProc::onPause() {
		return detail::c_pauseDefault;
	}
	void IMainProc::onResume() {}
	void IMainProc::onStop() {}
	void IMainProc::onReStart() {}
}
