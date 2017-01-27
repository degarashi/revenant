#include "main.hpp"
#include "../../input.hpp"
#include "../../gl_resource.hpp"
#include "../../sharedata.hpp"
#include "../../font.hpp"
#include "../../util/sys_unif.hpp"
#include "../../systeminfo.hpp"
#include <boost/format.hpp>
#include "../../drawtoken/clear.hpp"

namespace rev {
	namespace test {
		const auto Id = IEffect::GlxId::GenTechId("Text", "Default");
		struct MyScene::St_None: StateT<St_None> {
			void onEnter(MyScene& self, ObjTypeId_OP) override {
				self._actQ = mgr_input.makeAction("quit");
				mgr_input.addAction(self._actQ);
				auto hKb = Keyboard::OpenKeyboard();
				self._actQ->addLink(hKb, InputFlag::Button, SDL_SCANCODE_ESCAPE);

				CCoreID cid(16, 0, CCoreID::CharFlag_AA, false, 100, CCoreID::SizeType_Pixel);
				cid = mgr_text.makeCoreID("IPAGothic", cid);
				self._fps.setCCoreId(cid);
				self._fps.setDepth(0);
				self._fps.setWindowOffset({10,0});
			}
			void onUpdate(MyScene& self) override {
				self._fps.setText((boost::format("FPS: %1%") % mgr_info.getFPS()).str().c_str());
				if(self._actQ->isKeyPressed())
					mgr_scene.setPopScene(1);
			}
			void onDraw(const MyScene& self, IEffect& e) const override {
				auto& fx = static_cast<util::GLEffect_2D3D&>(e);
				static float a=0;
				a += 0.02f;
				e.clearFramebuffer({frea::Vec4{std::sin(a)/2+0.5f,std::sin(a*1.3)/2+0.5,0,0}, 1.f, 0});
				fx.setTechPassId(::rev::test::Id);
				self._fps.draw(fx);
			}
		};
		MyScene::MyScene() {
			setStateNew<St_None>();
		}
	}
}
