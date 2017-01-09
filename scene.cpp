#include "scene.hpp"

namespace rev {
	// -------------------- IScene --------------------
	HGroup IScene::getUpdGroup() const {
		D_Expect(false, "invalid function call");
		return HGroup();
	}
	HDGroup IScene::getDrawGroup() const {
		D_Expect(false, "invalid function call");
		return HDGroup();
	}
	// -------------- SceneBase --------------
	SceneBase::SceneBase(const HGroup& hUpd, const HDGroup& hDraw) {
		_update = hUpd ? hUpd : rev_mgr_obj.emplace<Update>();
		_draw = hDraw ? hDraw : rev_mgr_obj.emplace<Draw>();
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

	// -------------- SceneMgr --------------
	bool SceneMgr::isEmpty() const noexcept {
		return _scene.empty();
	}
	IScene& SceneMgr::getSceneInterface(const int n) const {
		return *getScene(n);
	}
	HScene SceneMgr::getTop() const {
		return getScene(0);
	}
	HScene SceneMgr::getScene(const int n) const {
		if(static_cast<int>(_scene.size()) > n)
			return _scene.at(_scene.size()-1-n);
		return HScene();
	}
	UpdGroup& SceneMgr::getUpdGroupRef(const int n) const {
		return *getSceneInterface(n).getUpdGroup();
	}
	DrawGroup& SceneMgr::getDrawGroupRef(const int n) const {
		return *getSceneInterface(n).getDrawGroup();
	}
	void SceneMgr::setPushScene(const HScene& hSc, const bool bPop) {
		if(_scene.empty()) {
			_scene.emplace_back(hSc);
			_scene.back()->onConnected(HGroup());
		} else {
			_scNext = hSc;
			_scNPop = bPop ? 1 : 0;
			_scArg = LCValue();
			_scOp = true;
		}
	}
	void SceneMgr::setPopScene(const int nPop, const LCValue& arg) {
		_scNext = HScene();
		_scNPop = nPop;
		_scArg = arg;
		_scOp = true;
	}
	void SceneMgr::_doSceneOp() {
		if(!_scOp && _scene.back()->isDead()) {
			// Sceneがdestroyされていて、かつSceneOpが無ければpop(1)とする
			setPopScene(1);
		}
		while(_scOp) {
			// Update中に指示されたScene操作タスクを実行
			ObjTypeId id = _scene.back()->getTypeId();

			int nPop = std::min(_scNPop, static_cast<int>(_scene.size()));
			_scNPop = 0;
			while(--nPop >= 0) {
				auto& sc = _scene.back();
				sc->onDisconnected(HGroup());
				sc->destroy();
				sc->onUpdate(true);			// nullステートへ移行させる
				_scene.pop_back();
			}
			// Sceneスタックが空ならここで終わり
			if(_scene.empty())
				return;

			_scOp = false;
			// 直後に積むシーンがあればそれを積む
			if(_scNext) {
				_scene.push_back(_scNext);
				_scNext->onConnected(HGroup());
				_scNext.reset();
			} else {
				// 降りた先のシーンに戻り値を渡す
				_scene.back()->onDown(id, _scArg);
				_scArg = LCValue();
			}
		}
	}
	bool SceneMgr::onUpdate() {
		UpdGroup::ProcAddRemove();
		if(_scene.empty())
			return false;

		_scene.back()->onUpdate(true);
		// SceneOpがあれば処理
		_doSceneOp();
		// スタックが空だったらtrue = 終了の合図 を返す
		return !_scene.empty();
	}
	void SceneMgr::onDraw(IEffect& e) {
		if(_scene.empty())
			return;

		_scene.back()->onDraw(e);
		// DrawフェーズでのSceneOpは許可されない
		D_Assert0(!_scOp);
	}
	#define DEF_ADAPTOR(ret, name) ret SceneMgr::name() { \
		return _scene.front()->name(); }
	DEF_ADAPTOR(void, onStop)
	DEF_ADAPTOR(void, onReStart)
	DEF_ADAPTOR(bool, onPause)
	DEF_ADAPTOR(void, onResume)
	#undef DEF_ADAPTOR

	// -------------- U_Scene --------------
	struct U_Scene::St_None : StateT<St_None> {};
	U_Scene::U_Scene() {
		setStateNew<St_None>();
	}
}
