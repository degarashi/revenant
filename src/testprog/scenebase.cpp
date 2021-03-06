#include "scenebase.hpp"
#include "../text/text.hpp"
#include "../input/input.hpp"
#include "../effect/if.hpp"
#include "../systeminfo.hpp"
#include "../effect/clear.hpp"
#include <boost/format.hpp>

namespace rev::test {
	TestScene::TestScene() {
		_textTech = rev::detail::TextObj::GetDefaultTech();
		{
			// フォント初期化
			FontId fid(16, 0, FontId::CharFlag_AA, false, 100, FontId::SizeType_Pixel);
			fid = mgr_text.appendFaceId("IPAGothic", fid);
			_fps.setFontId(fid);
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
		}
	}
	void TestScene::_showFPS(IEffect& e) const {
		// FPSを左上に表示
		e.setTechnique(_textTech);
		_fps.setText((boost::format("FPS: %1%") % mgr_info.getFPS()).str().c_str());
		_fps.draw(e);
	}
	void TestScene::_clearBg(IEffect& e) const {
		// 適当に背景をベタ塗り
		e.clearFramebuffer({frea::Vec4{0.6,0.6,1,0}, 1.f, 0});
	}
}
