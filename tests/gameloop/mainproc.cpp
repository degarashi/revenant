#include "main.hpp"
#include "../../input.hpp"
#include "../../output.hpp"

namespace rev {
	namespace test {
		MainProc::MainProc() {
			_actQ = mgr_input.makeAction("quit");
			mgr_input.addAction(_actQ);
			auto hKb = Keyboard::OpenKeyboard();
			_actQ->addLink(hKb, InputFlag::Button, SDL_SCANCODE_ESCAPE);
		}
		bool MainProc::runU() {
			if(_actQ->isKeyPressed())
				return false;
			return true;
		}
		bool MainProc::onPause() {
			LogR(Verbose, "OnPause");
			return true;
		}
		void MainProc::onResume() {
			LogR(Verbose, "OnResume");
		}
		void MainProc::onStop() {
			LogR(Verbose, "OnStop");
		}
		void MainProc::onReStart() {
			LogR(Verbose, "OnRestart");
		}
	}
}
