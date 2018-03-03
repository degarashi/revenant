#include "updtask.hpp"
#include "lubee/sort.hpp"

namespace rev {
	namespace {
		const std::string cs_updtaskname("UpdTask");
	}
	// -------------------- UpdTask --------------------
	UpdTask::UpdTask(Priority /*p*/, const HGroup& hGroup):
		_idleCount(0),
		_accum(0),
		_hGroup(hGroup)
	{}
	bool UpdTask::isNode() const {
		return true;
	}
	void UpdTask::onConnected(const HGroup& hGroup) {
		_hGroup->onConnected(hGroup);
	}
	void UpdTask::onDisconnected(const HGroup& hGroup) {
		_hGroup->onDisconnected(hGroup);
	}
	void UpdTask::enumGroup(const CBFindGroup& cb, const GroupTypeId id, const int depth) const {
		_hGroup->enumGroup(cb, id, depth);
	}
	void UpdTask::proc(const CBUpdProc& p, const bool bRecursive, const Priority prioBegin, const Priority prioEnd) {
		_hGroup->proc(p, bRecursive, prioBegin, prioEnd);
	}
	bool UpdTask::recvMsg(LCValue& dst, const GMessageStr& msg, const LCValue& arg) {
		return _hGroup->recvMsg(dst, msg, arg);
	}
	void UpdTask::onUpdate(bool /*bFirst*/) {
		// アイドル時間チェック
		if(_idleCount > 0)
			--_idleCount;
		else
			_hGroup->onUpdate(true);
		++_accum;
	}
	void UpdTask::setIdle(int nFrame) {
		_idleCount = nFrame;
	}
	int UpdTask::getAccum() const {
		return _accum;
	}
	const char* UpdTask::getResourceName() const noexcept {
		return cs_updtaskname.c_str();
	}
}
