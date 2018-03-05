#include "updtask.hpp"
#include "lubee/sort.hpp"

namespace rev {
	UpdTask::UpdTask(Priority p):
		UpdGroup(p),
		_idleCount(0),
		_accum(0)
	{}
	void UpdTask::onUpdate(const bool execLua) {
		// アイドル時間チェック
		if(_idleCount > 0)
			--_idleCount;
		else if(_idleCount == 0)
			UpdGroup::onUpdate(execLua);;
		++_accum;
	}
	void UpdTask::setIdle(const int nFrame) {
		_idleCount = nFrame;
	}
	int UpdTask::getAccum() const {
		return _accum;
	}
}
