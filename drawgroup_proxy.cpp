#include "drawgroup_proxy.hpp"
#include "drawgroup.hpp"

namespace rev {
	DrawGroupProxy::DrawGroupProxy(const HDGroup& hDg, const Priority prio):
		_hDGroup(hDg)
	{
		setPriority(prio);
	}
	void DrawGroupProxy::onUpdate(const bool bFirst) {
		// DrawGroupのonUpdateを呼ぶとエラーになるが、一応呼び出し
		_hDGroup->onUpdate(bFirst);
	}
	void DrawGroupProxy::setPriority(const Priority p) noexcept {
		_dtag.priority = p;
	}
	const DSort_V& DrawGroupProxy::getSortAlgorithm() const noexcept {
		return _hDGroup->getSortAlgorithm();
	}
	const DLObj_V& DrawGroupProxy::getMember() const noexcept {
		return _hDGroup->getMember();
	}
	bool DrawGroupProxy::isNode() const noexcept {
		return true;
	}
	void DrawGroupProxy::onDraw(IEffect& e) const {
		_hDGroup->onDraw(e);
	}
	DrawTag& DrawGroupProxy::refDTag() noexcept {
		return _dtag;
	}
	const char* DrawGroupProxy::getResourceName() const noexcept {
		return _hDGroup->getResourceName();
	}
}
