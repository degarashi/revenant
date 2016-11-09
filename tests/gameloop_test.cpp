#include "test.hpp"
#include "../gameloopparam.hpp"
#include "../drawproc.hpp"
#include "../mainproc.hpp"
#include "../gameloop.hpp"
#include "../glx.hpp"
#include "../gl_resource.hpp"
#include "../output.hpp"
#include "../input.hpp"

namespace rev {
	namespace test {
		struct DrawProc : IDrawProc {
			bool runU(const uint64_t /*accum*/, const bool /*bSkip*/) override {
				GL.glClearColor(0.5f,0,0,0);
				GL.glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
				return true;
			}
		};
		struct MainProc : IMainProc {
			HAct _actQ;
			MainProc() {
				_actQ = mgr_input.makeAction("quit");
				mgr_input.addAction(_actQ);
				auto hKb = Keyboard::OpenKeyboard();
				_actQ->addLink(hKb, InputFlag::Button, SDL_SCANCODE_ESCAPE);
			}
			bool runU(Query& /*q*/) override {
				if(_actQ->isKeyPressed())
					return false;
				return true;
			}
			bool onPause() override {
				LogR(Verbose, "OnPause");
				return true;
			}
			void onResume() override {
				LogR(Verbose, "OnResume");
			}
			void onStop() override {
				LogR(Verbose, "OnStop");
			}
			void onReStart() override {
				LogR(Verbose, "OnRestart");
			}
		};
		struct Param : GameloopParam {
			lubee::SizeI getScreenSize() const override {
				return {640, 480};
			}
			//! アプリケション名(一時ファイル用)
			const std::string& getAppName() const override {
				static std::string name("App_Name");
				return name;
			}
			//! 組織名(一時ファイル用)
			const std::string& getOrgName() const override {
				static std::string name("Org_Name");
				return name;
			}
			//! パス記述ファイル名
			const URI& getPathfile() const override {
				static URI uri("file", "./pathlist");
				return uri;
			}
			HFx loadEffect(const std::string& name) const override {
				return mgr_gl.template loadEffect<GLEffect>(name);
			}
			IMainProc* makeMainProc(const Window_SP&) const override {
				return new MainProc();
			}
			IDrawProc* makeDrawProc() const override {
				return new DrawProc();
			}
			bool getMultiContext() const noexcept override {
				return true;
			}
		};
		struct GameloopTest : Random {};
		TEST_F(GameloopTest, General) {
			GUIThread loop(std::make_unique<Param>());
			loop.run();
		}
	}
}
