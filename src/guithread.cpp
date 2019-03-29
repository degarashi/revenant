#include "guithread.hpp"
#include "gameloopparam.hpp"
#include "msg/system.hpp"
#include "mainthread.hpp"
#include "sdl/init.hpp"
#include "sdl/window.hpp"
#include "input/input.hpp"
#include "log/pooled_output.hpp"
#include "output.hpp"
#include "sharedata.hpp"
#include "log/time.hpp"
#include "handle/sdl.hpp"
#include <SDL.h>
#include <SDL_image.h>

namespace rev {
	const uint32_t EVID_SIGNAL = SDL_RegisterEvents(1);
	// ---------------- GUIThread ----------------
	GUIThread::GUIThread(GameloopParam_UP param):
		_level(Level::Active)
	{
		g_system_shared.lock()->param = std::move(param);
	}
	int GUIThread::run() {
		SDLInitializer	sdlI(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_TIMER);
		IMGInitializer	imgI(IMG_INIT_JPG | IMG_INIT_PNG);
		HWin window;
		// アプリケーション開始時刻を記録
		RecordApplicationBeginTime();
		tls_threadID = SDL_GetThreadID(nullptr);
		tls_threadName = "GuiThread";
		#ifdef ANDROID
			// egl関数がロードされてないとのエラーが出る為
			SDL_GL_LoadLibrary("libEGL.so");
		#endif
		lubee::log::Log::GetOutput() = std::make_unique<log::PooledRevOutput>();
		LogR(Verbose, "RevOutput initializing done");
		{
			Window::GLParam gp;
			gp.verMajor = 2;
			gp.verMinor = 0;
			gp.red = gp.green = gp.blue = 8;
			gp.depth = 16;
			gp.setStdAttributes();
			LogR(Verbose, "OpenGL Param(Request):\n%1%", gp);
		}
		{
			auto lk = g_system_shared.lock();
			Window::Param wp;
			wp.title = lk->param->getAppName() + " by RVE";
			wp.size = lk->param->getScreenSize();
			wp.flag = SDL_WINDOW_SHOWN;
			LogR(Verbose, "%1%", wp);

			window = Window::Create(wp);
			SDLMouse::SetWindow(window->getWindow());
			lk->window = window;
			LogR(Verbose, "Window created");
		}
		// メインスレッドのメッセージキューを初期化
		Looper::Prepare();
		auto& loop = Looper::GetLooper();
		// メインスレッドに渡す
		MainThread mth;
		mth.start(std::ref(loop));
		// メインスレッドのキューが準備出来るのを待つ
		while(auto msg = loop->wait()) {
			if(static_cast<msg::MainInit*>(*msg))
				break;
			else if(static_cast<msg::QuitReq*>(*msg))
				break;
		}
		_handler = Handler(mth.getLooper());
		LogR(Verbose, "Mainthread initialized");
		LogR(Verbose, "Entering loop...");
		// GUIスレッドのメッセージループ
		SDL_Event e;
		bool bLoop = true;
		while(bLoop && mth.isRunning() && SDL_WaitEvent(&e)) {
			if(e.type == EVID_SIGNAL) {
				// 自作スレッドのキューにメッセージがある
				while(auto m = loop->peek()) {
					if(static_cast<msg::QuitReq*>(*m)) {
						e.type = SDL_QUIT;
						e.quit.timestamp = 0;
						SDL_PushEvent(&e);
					}
				}
			} else {
				PrintEvent::All(e);
				// 当分はGame_Sleep()でリソース解放、Game_Wakeup()でリソース復帰とする
				// (onStop()やonStart()は関知しない)
				switch(e.type) {
					case SDL_WINDOWEVENT:
						_procWindowEvent(e);
						break;
					case SDL_MOUSEWHEEL:
						_procMouseWheel(e);
						break;
					case SDL_MOUSEBUTTONDOWN:
						_procMouseButtonDown(e);
						break;
					case SDL_KEYDOWN:
					case SDL_KEYUP:
						_procKey(e);
						break;
					case SDL_TEXTINPUT:
						_procTextInput(e);
						break;
					case SDL_QUIT:
						// アプリケーション終了コールが来たらループを抜ける
						bLoop = false;
						break;
					case SDL_APP_TERMINATING:			// Android: onDestroy()
						break;
					case SDL_APP_LOWMEMORY:				// Android: onLowMemory()
						break;
					case SDL_APP_WILLENTERBACKGROUND:	// Android: onPause()
					case SDL_APP_DIDENTERBACKGROUND:	// Android: onPause()
						_setLevel(Level::e(std::min<int>(_level, Level::Stop)));
						break;
					case SDL_APP_WILLENTERFOREGROUND:	// Android: onResume()
					case SDL_APP_DIDENTERFOREGROUND:	// Android: onResume()
						_setLevel(Level::e(std::max<int>(_level, Level::Active)));
						break;
				}
			}
		}
		LogR(Verbose, "Exited loop");
		// 万が一メインスレッドがまだ終了してない際は強制終了させる
		mth.interrupt();
		try {
			// 例外が投げられて終了したかをチェック
			mth.getResult();
		} catch (...) {
			LogR(Verbose, "main thread was ended by throwing exception");
			return 1;
		}
		LogR(Verbose, "Ended");
		return 0;
	}
	void GUIThread::_procWindowEvent(SDL_Event& e) {
		switch(e.window.event) {
			case SDL_WINDOWEVENT_CLOSE:
				// ウィンドウ閉じたら終了
				e.type = SDL_QUIT;
				e.quit.timestamp = 0;
				SDL_PushEvent(&e);
				break;
			case SDL_WINDOWEVENT_MINIMIZED:
				_setLevel(Level::e(std::min<int>(_level, Level::Stop)));
				break;
			case SDL_WINDOWEVENT_RESTORED:
				_setLevel(Level::e(std::max<int>(_level, Level::Pause)));
				break;
			case SDL_WINDOWEVENT_FOCUS_GAINED:
				_setLevel(Level::e(std::max<int>(_level, Level::Active)));
				break;
			case SDL_WINDOWEVENT_FOCUS_LOST:
				_setLevel(Level::e(std::min<int>(_level, Level::Pause)));
				break;
			default: break;
		}
	}
	// それぞれユーザースレッドに通知
	void GUIThread::_onPause() {
		_handler->postMessageNow(msg::PauseReq());
	}
	void GUIThread::_onResume() {
		_handler->postMessageNow(msg::ResumeReq());
	}
	void GUIThread::_onStop() {
		_handler->postMessageNow(msg::StopReq());
	}
	void GUIThread::_onReStart() {
		_handler->postMessageNow(msg::ReStartReq());
	}
	void GUIThread::_setLevel(const Level level) {
		int ilevel = level,
			curLevel = _level;
		const int idx = ilevel > curLevel ? 0 : 1,
					inc = ilevel > curLevel ? 1 : -1;
		while(ilevel != curLevel) {
			if(LFunc f = cs_lfunc[curLevel][idx])
				(this->*f)();
			curLevel += inc;
		}
		_level = level;
	}
	const GUIThread::LFunc GUIThread::cs_lfunc[Level::_Num][2] = {
		{&GUIThread::_onReStart, nullptr},
		{&GUIThread::_onResume, &GUIThread::_onStop},
		{nullptr, &GUIThread::_onPause}
	};
}
#include "input/sdlvalue.hpp"
#include "lubee/src/meta/countof.hpp"
namespace rev {
	void GUIThread::_procMouseWheel(SDL_Event& e) {
		D_Assert0(e.type == SDL_MOUSEWHEEL);
		if(e.wheel.which != SDL_TOUCH_MOUSEID) {
			auto lc = g_sdlInputShared.lock();
			lc->wheel_dx += e.wheel.x;
			lc->wheel_dy += e.wheel.y;
		}
	}
	void GUIThread::_procMouseButtonDown(SDL_Event& e) {
		D_Assert0(e.type == SDL_MOUSEBUTTONDOWN);
		const auto btn = e.button.button;
		for(std::size_t i=0 ; i<countof(SDLInputShared::c_buttonId) ; i++) {
			if(btn == SDLInputShared::c_buttonId[i]) {
				auto lc = g_sdlInputShared.lock();
				lc->button[i] = true;
				break;
			}
		}
	}
	void GUIThread::_procKey(SDL_Event& e) {
		D_Assert0(e.type==SDL_KEYDOWN || e.type==SDL_KEYUP);
		const bool down = (e.type == SDL_KEYDOWN);
		auto lc = g_sdlInputShared.lock();
		auto& inp = *lc;
		const int code = e.key.keysym.scancode;
		const int key = e.key.keysym.sym & ~SDLK_SCANCODE_MASK;
		inp.key.emplace_back(KeyLog{code, key, down});
		inp.keyaux.shift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
		inp.keyaux.ctrl = ((SDL_GetModState() & KMOD_CTRL) != 0);
		inp.keyaux.alt = ((SDL_GetModState() & KMOD_ALT) != 0);
		inp.keyaux.super = ((SDL_GetModState() & KMOD_GUI) != 0);
	}
	void GUIThread::_procTextInput(SDL_Event& e) {
		D_Assert0(e.type==SDL_TEXTINPUT);
		auto lc = g_sdlInputShared.lock();
		lc->text.append(e.text.text);
	}
}
