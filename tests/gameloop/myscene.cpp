#include "myscene.hpp"
#include "../../font.hpp"
#include "../../input.hpp"
#include "../../scene_mgr.hpp"
#include "../../glx_if.hpp"
#include "../../systeminfo.hpp"
#include "../../clear.hpp"
#include <boost/format.hpp>
#ifdef DEBUGGUI_ENABLED
	#include "mygui.hpp"
#endif

namespace rev::test {
	MyScene::MyScene():
		_sceneId(2)
	{
		_textTech = rev::TextObj::GetDefaultTech();
		{
			// フォント初期化
			CCoreID cid(16, 0, CCoreID::CharFlag_AA, false, 100, CCoreID::SizeType_Pixel);
			cid = mgr_text.makeCoreID("IPAGothic", cid);
			_fps.setCCoreId(cid);
			_fps.setDepth(0);
			_fps.setWindowOffset({10,30});
		}
		{
			const auto hKb = Keyboard::OpenKeyboard();
			// 終了ボタン定義
			_act[Act::Quit] = mgr_input.makeAction("quit");
			_act[Act::Quit]->addLink(hKb, InputFlag::Button, VKey::Escape);
			// ポーズボタン定義
			_act[Act::Pause] = mgr_input.makeAction("pause");
			_act[Act::Pause]->addLink(hKb, InputFlag::Button, VKey::F1);
			// シーン切り替えボタン定義
			_act[Act::SceneSwitch] = mgr_input.makeAction("switch");
			_act[Act::SceneSwitch]->addLink(hKb, InputFlag::Button, VKey::F2);
		}
		_setSceneById(_sceneId);
		_makeGui();
		auto dg = getDrawGroup();
		dg->setSortAlgorithm({cs_dsort_priority_desc, cs_dsort_texture}, true);
	}
	void MyScene::_setSceneById(const std::size_t id) {
		if(id == 0)
			setStateNew<St_Sprite>();
		else if(id == 1)
			setStateNew<St_Cube>();
		else
			setStateNew<St_glTF>();
	}
	void MyScene::_checkQuit() {
		if(_act[Act::Quit]->isKeyPressed())
			mgr_scene.setPopScene(1);
	}
	void MyScene::_checkPause() {
		if(_act[Act::Pause]->isKeyPressed()) {
			auto upd = getBase().getUpdate();
			auto& w = upd->refWait();
			w.wait = ~w.wait;
		}
	}
	void MyScene::_checkSwitch() {
		if(_act[Act::SceneSwitch]->isKeyPressed()) {
			_act[Act::SceneSwitch]->cancelPressed();
			_sceneId = (_sceneId+1) % NState;
			_setSceneById(_sceneId);
		}
	}
	void MyScene::_showFPS(IEffect& e) const {
		// FPSを左上に表示
		e.setTechnique(_textTech);
		_fps.setText((boost::format("FPS: %1%") % mgr_info.getFPS()).str().c_str());
		_fps.draw(e);
	}
	void MyScene::_clearBg(IEffect& e) const {
		// 適当に背景をベタ塗り
		e.clearFramebuffer({frea::Vec4{0.6,0.6,1,0}, 1.f, 0});
	}
	void MyScene::_makeGui() {
		#ifdef DEBUGGUI_ENABLED
			auto& ug = getUpdGroupRef();
			ug.addObj(std::make_shared<MyGUI>(getDrawGroup()));
		#endif
	}

	void MyScene::St_Base::onUpdate(MyScene& self) {
		self._checkQuit();
		self._checkPause();
		self._checkSwitch();
	}
	void MyScene::St_Base::onDraw(const MyScene& self, IEffect& e) const {
		self._clearBg(e);
		self._showFPS(e);
	}
}
