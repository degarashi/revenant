#include "scene_base.hpp"
#include "object_mgr.hpp"

namespace rev {
	SceneBase::SceneBase(const HGroup& hUpd, const HDGroup& hDraw) {
		_update = hUpd ? hUpd : rev_mgr_obj.emplace<UpdGroup>();
		_draw = hDraw ? hDraw : rev_mgr_obj.emplace<DrawGroup>();
	}
	void SceneBase::setUpdate(const HGroup& hGroup) {
		if(_update)
			_update->onDisconnected(HGroup());
		_update = hGroup;
		if(hGroup)
			hGroup->onConnected(HGroup());
	}
	const HGroup& SceneBase::getUpdate() const noexcept {
		return _update;
	}
	void SceneBase::setDraw(const HDGroup& hdGroup) {
		// DrawGroupはConnected系の通知を行わない
		_draw = hdGroup;
	}
	const HDGroup& SceneBase::getDraw() const noexcept {
		return _draw;
	}
}
