#include "updtask.hpp"
#include "lubee/sort.hpp"

namespace rev {
	UpdTask::UpdTask(Priority p):
		UpdGroup(p),
		_interval(interval::Wait(0))
	{}
	void UpdTask::onUpdate(const bool execLua) {
		if(_interval.advance())
			UpdGroup::onUpdate(execLua);
	}
	void UpdTask::setIdle(const int nFrame) {
		std::get<interval::Wait>(_interval).wait = nFrame;
	}
	int UpdTask::getIdle() const noexcept {
		return std::get<interval::Wait>(_interval).wait;
	}
	int UpdTask::getAccum() const noexcept {
		return _interval.accum;
	}
}
