#include "drawthread.hpp"
#include "sys_message.hpp"
#include "gl_resource.hpp"
#include "gl_if.hpp"
#include "output.hpp"
#include "sharedata.hpp"
#include "gameloopparam.hpp"
#include "sdl_glctx.hpp"
#include "handler.hpp"

namespace rev {
	// --------------------- DrawThread ---------------------
	DrawThread::DrawThread():
		ThreadL("DrawThread")
	{}
	void DrawThread::runL(const Looper_SP& mainLooper) {
		const bool MultiContext = g_system_shared.lock()->param->getMultiContext();
		try {
			const auto hasContext = [this](){
				return static_cast<bool>(_info.lock()->ctxDrawThread);
			};
			// GLコンテキスト確保
			/*!
				\param[in] bForce	trueの時は既にコンテキストが確保されていても再確保する
				\return true		コンテキストが確保された時はtrue
			*/
			const auto fnMakeContext = [this, MultiContext](const bool bForce){
				auto lk = _info.lock();
				auto& ctxD = lk->ctxDrawThread;
				auto& ctxM = lk->ctxMainThread;
				if(bForce || !ctxD) {
					auto glk = g_system_shared.lock();
					auto w = glk->window.lock();
					ctxD = GLContext::CreateContext(w, false);
					ctxD->makeCurrent(w);
					if(MultiContext) {
						ctxM = GLContext::CreateContext(w, true);
						ctxD->makeCurrent(w);
					}
					return true;
				}
				return false;
			};
			// (まだ破棄されていなければ)GLコンテキストを破棄
			const auto fnDestroyContext = [this, MultiContext](){
				auto lk = _info.lock();
				if(lk->ctxDrawThread) {
					if(MultiContext)
						lk->ctxMainThread.reset();
					lk->ctxDrawThread.reset();
				}
			};

			fnMakeContext(true);
			Handler mainHandler(mainLooper);
			Handler drawHandler(Looper::GetLooper());
			GLW.initializeDrawThread(drawHandler);
			GLW.loadGLFunc();
			// ここで一旦MainThreadにOpenGLコンテキストの初期化が終わったことを通知
			mainHandler.postArgs(msg::DrawInit());
			DrawProc_UP up(g_system_shared.lock()->param->makeDrawProc());

			LogR(Verbose, "Entering loop.");
			bool bLoop = true;
			for(;;) {
				// メインスレッドから描画開始の指示が来るまで待機
				// AndroidではContextSharingが出来ないのでメインスレッドからロードするタスクを受け取ってここで処理
				while(auto m = getLooper()->wait()) {
					if(msg::DrawReq* p = *m) {
						// -- 描画リクエスト --
						// ステート値をDrawingへ変更
						_info.lock()->state = State::Drawing;
						// 1フレーム分の描画処理
						GL.setSwapInterval(0);
						if(up->runU(p->id, p->bSkip)) {
							// 描画スキップされてなければFPSカウンタを更新
							auto lk = _info.lock();
							lk->fps.update();
							lk->ctxDrawThread->swapWindow();
						}
						GL.glFlush();
						{
							// ステート値をIdleへ戻し、累積描画フレーム数を更新
							auto lk = _info.lock();
							lk->state = State::Idle;
							lk->accum = p->id;
						}
					} else if(static_cast<msg::QuitReq*>(*m)) {
						// -- スレッド終了リクエスト --
						bLoop = false;
						break;
					} else if(static_cast<msg::MakeContext*>(*m)) {
						LogR(Verbose, "MakeContext");
						// -- OpenGLコンテキストを(再度)作成 --
						// いったんOpenGLリソースを解放
						if(hasContext())
							mgr_gl.onDeviceLost();
						GL.glFlush();
						fnMakeContext(false);
						// OpenGLリソースの再確保
						mgr_gl.onDeviceReset();
					} else if(static_cast<msg::DestroyContext*>(*m)) {
						LogR(Verbose, "DestroyContext");
						// -- OpenGLコンテキストを破棄 --
						Expect(hasContext(), "No OpenGL context available");
						mgr_gl.onDeviceLost();
						GL.glFlush();
						fnDestroyContext();
					}
				}
				if(isInterrupted()) {
					LogR(Verbose, "Exiting loop by interrupting");
					break;
				}
				if(!bLoop) {
					LogR(Verbose, "Exiting loop normally");
					break;
				}
			}
			up.reset();
			LogR(Verbose, "Drawproc deleted");
			// ステート値をEndedへ変更
			_info.lock()->state = State::Ended;
			// 後片付けフェーズ
			LogR(Verbose, "Destructor begun");
			bLoop = true;
			// interruptされたのでなければ二回目のQuitReqを待つ
			while(bLoop && !isInterrupted()) {
				while(auto m = getLooper()->wait()) {
					if(static_cast<msg::QuitReq*>(*m)) {
						bLoop = false;
						break;
					}
				}
			}
			fnDestroyContext();
			GLW.terminateDrawThread();
			LogR(Verbose, "Destructor ended");
		} catch(const std::exception& e) {
			LogRHere(Error, "Exception thrown\ntype: %1%\nwhat: %2%", typeid(e).name(), e.what());
			throw;
		}
	}
}
