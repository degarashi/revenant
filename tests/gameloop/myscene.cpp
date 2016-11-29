#include "main.hpp"
#include "../../input.hpp"
#include "../../gl_resource.hpp"
#include "../../sharedata.hpp"
#include "../../sdl_mutex.hpp"
#include "../../font.hpp"
#include "../../glx_if.hpp"
#include "../../util/text2d.hpp"

namespace rev {
	namespace test {
		struct MyScene::St_None: StateT<St_None> {
			void onEnter(MyScene& self, ObjTypeId_OP) override {
				self._actQ = mgr_input.makeAction("quit");
				mgr_input.addAction(self._actQ);
				auto hKb = Keyboard::OpenKeyboard();
				self._actQ->addLink(hKb, InputFlag::Button, SDL_SCANCODE_ESCAPE);
			}
			void onUpdate(MyScene& self) override {
				auto lk = g_system_shared.lock();
				if(auto fx = lk->fx.lock()) {
					fx->beginTask();
					fx->endTask();
				}
				if(self._actQ->isKeyPressed())
					mgr_scene.setPopScene(1);
			}
		};
		MyScene::MyScene() {
			setStateNew<St_None>();
		}
	}
}
