#include "mainthread.hpp"
#include "drawthread.hpp"
#include "input.hpp"
#include "input_sdlvalue.hpp"
#include "systeminfo.hpp"
#include "sharedata.hpp"
#include "sdl_window.hpp"
#include "output.hpp"
#include "gl_if.hpp"
#include "handler.hpp"
#include "sys_message.hpp"
#include "sound.hpp"

namespace rev {
	bool MainThread::_updateFrame(MainProc* mp, DrawThread& dth, Handler& drawHandler) {
		// プロファイラのフレーム切り替え
		// spn::profiler.onFrame();
		try {
			// ゲーム進行
			++getInfo()->accumUpd;
			mgr_input.update();
			mgr_sound.update();
			g_sdlInputShared.lock()->reset();
			{
				mgr_info.setInfo(
					g_system_shared.lock()->window.lock()->getSize(),
					dth.getInfo()->fps.getFPS()
				);
				if(!mp->runU()) {
					LogR(Verbose, "Exiting loop by normally");
					return true;
				}
			}
			GL.glFlush();
			drawHandler.postMessageNow(msg::DrawReq(++getInfo()->accumDraw));
		} catch(const std::exception& e) {
			LogR(Error, "RunU() exception\n%s", e.what());
			throw;
		} catch(...) {
			LogR(Error, "RunU() unknown exception");
			throw;
		}
		if(isInterrupted()) {
			LogR(Verbose, "Exiting loop by interrupt");
			return true;
		}
		return false;
	}
}
