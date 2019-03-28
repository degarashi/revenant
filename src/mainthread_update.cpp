#include "resmgr_app.hpp"
#include "mainthread.hpp"
#include "drawthread.hpp"
#include "input/input.hpp"
#include "input/sdlvalue.hpp"
#include "systeminfo.hpp"
#include "sharedata.hpp"
#include "sdl/window.hpp"
#include "output.hpp"
#include "gl/if.hpp"
#include "msg/handler.hpp"
#include "msg/system.hpp"
#include "sound/sound.hpp"

namespace rev {
	bool MainThread::_updateFrame(MainProc* mp, DrawThread& dth, Handler& drawHandler, const Duration delta) {
		try {
			{
				// ゲーム進行
				const auto ac = ++getInfo()->accumUpd;
				// 16フレームに一度、FlyweightItemのGCを呼ぶ
				if((ac & 0x0f) == 0)
					this->_FlyweightGC();
			}
			mgr_input.update();
			mgr_sound.update();
			g_sdlInputShared.lock()->reset();
			{
				{
					auto lc = g_system_shared.lockC();
					const auto w = lc->window.lock();
					mgr_info.setInfo(
						w->getSize(),
						dth.getInfo()->fps.getFPS()
					);
				}
				if(!mp->runU(delta)) {
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
