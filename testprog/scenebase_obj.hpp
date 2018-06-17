#pragma once
#include "scenebase.hpp"
#include "mygui.hpp"
#include "../scene.hpp"
#include "../scene_mgr.hpp"
#include "../input.hpp"

namespace rev::test {
	template <class Der>
	class TestSceneObj :
		public Scene<Der>,
		public TestScene
	{
		private:
			using base_t = Scene<Der>;
			HObj					_gui;

			void _checkQuit() {
				if(_act[Act::Quit]->isKeyPressed())
					mgr_scene.setPopScene(1);
			}
			void _checkPause() {
				if(_act[Act::Pause]->isKeyPressed()) {
					auto upd = this->getBase().getUpdate();
					auto& w = upd->refWait();
					w.wait = ~w.wait;
				}
			}
		protected:
			void _makeGui() {
				auto& ug = this->getUpdGroupRef();
				ug.addObj(std::make_shared<MyGUI>(this->getDrawGroup()));
			}
			struct St_Base : base_t::template StateT<St_Base> {
				void onUpdate(Der& self) override {
					self._checkQuit();
					self._checkPause();
				}
				void onDraw(const Der& self, IEffect& e) const override {
					self._clearBg(e);
					self._showFPS(e);
				}
			};
			TestSceneObj() {
				const auto dg = this->getDrawGroup();
				dg->setSortAlgorithm({cs_dsort_priority_desc, cs_dsort_texture}, true);
			}
	};
}
