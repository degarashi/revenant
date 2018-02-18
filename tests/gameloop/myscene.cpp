#include "main.hpp"
#include "../../input.hpp"
#include "../../glx_if.hpp"
#include "../../gl_resource.hpp"
#include "../../gl_texture.hpp"
#include "../../sharedata.hpp"
#include "../../font.hpp"
#include "../../systeminfo.hpp"
#include <boost/format.hpp>
#include "../../drawtoken/clear.hpp"
#include "frea/random/vector.hpp"
#include "sprite2d.hpp"
#include "bsprite.hpp"
#include "../../camera2d.hpp"
#include "../../sys_uniform_value.hpp"
#include "../../sys_uniform.hpp"
#include "../../tech_pass.hpp"
#include "../../output.hpp"

namespace rev {
	namespace test {
		struct MyScene::St_None: StateT<St_None> {
			void onEnter(MyScene& self, ObjTypeId_OP) override {
				{
					auto lk = g_shared.lock();
					lk->technique = mgr_gl.loadTechPass("sprite2d.glx");
					lk->textTech = rev::TextObj::GetDefaultTech();
				}
				// 終了ボタン定義
				self._actQ = mgr_input.makeAction("quit");
				mgr_input.addAction(self._actQ);
				auto hKb = Keyboard::OpenKeyboard();
				self._actQ->addLink(hKb, InputFlag::Button, SDL_SCANCODE_ESCAPE);

				// カメラ初期化
				self._camera = mgr_cam2d.emplace();
				auto scr = mgr_info.getScreenSize();
				self._camera->setAspectRatio(scr.width / scr.height);

				// フォント初期化
				CCoreID cid(16, 0, CCoreID::CharFlag_AA, false, 100, CCoreID::SizeType_Pixel);
				cid = mgr_text.makeCoreID("IPAGothic", cid);
				self._fps.setCCoreId(cid);
				self._fps.setDepth(0);
				self._fps.setWindowOffset({10,0});

				auto& ug = self.getUpdGroupRef();
				auto& dg = self.getDrawGroupRef();
				auto lk = g_system_shared.lock();
				auto rfi = lk->mt.getUniformF<int>();
				auto rf = lk->mt.getUniformF<float>();
				HTex tex[6];
				for(uint32_t i=0 ; i<countof(tex) ; i++) {
					tex[i] = mgr_gl.loadTexture(UserURI((boost::format("spr%1%.png") % i).str()), MipState::MipmapLinear);

					tex[i]->setFilter(true, true);
				}
				using frea::random::GenVecUnit;
				using frea::random::GenVec;
				for(int i=0 ; i<16 ; i++) {
					const auto vDir = GenVecUnit<Vec2>(rf);
					const auto vPos = GenVec<Vec2>(rf, {-1.f, 1.f});
					auto obj = std::make_shared<BoundingSprite>(tex[rfi({0,5})], vPos, vDir*0.01f);
					obj->setScaling(Vec2{0.2,0.2});
					ug.addObj(obj);
					dg.addObj(obj);
				}
				auto& cp = self._camera->refPose();
				cp.identity();
			}
			void onUpdate(MyScene& self) override {
				// 終了ボタン判定
				if(self._actQ->isKeyPressed())
					mgr_scene.setPopScene(1);
			}
			void onDraw(const MyScene& self, IEffect& e) const override {
				// 適当に背景をベタ塗り
				static float a=0;
				a += 0.02f;
				e.clearFramebuffer({frea::Vec4{std::sin(a)/2+0.5f,std::sin(a*1.3)/2+0.5,0,0}, 1.f, 0});
				// FPSを左上に表示
				auto& fps = const_cast<decltype(self._fps)&>(self._fps);
				{
					auto lc = g_shared.lockC();
					e.setTechnique(lc->textTech);
				}
				fps.setText((boost::format("FPS: %1%") % mgr_info.getFPS()).str().c_str());
				fps.draw(e);
				auto& e2 = e.ref2D();
				e2.setCamera(self._camera);
			}
		};
		MyScene::MyScene() {
			setStateNew<St_None>();
		}
		MyScene::~MyScene() {
			auto lk = g_shared.lock();
			lk->technique.reset();
			lk->textTech.reset();
		}
	}
}
