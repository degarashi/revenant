#include "mainthread.hpp"
#include "sharedata.hpp"
#include "gameloopparam.hpp"
#include "gl_if.hpp"
#include "drawthread.hpp"
#include "sys_message.hpp"
#include "handler.hpp"
#include "sdl_glctx.hpp"
#include "mainproc.hpp"
#include "systeminfo.hpp"
#include "output.hpp"
#include "sdl_window.hpp"
#include "sound.hpp"
#include <SDL_timer.h>
#include <SDL_events.h>
#include "videoparam.hpp"
#include "scene.hpp"
#include "watch.hpp"
#include "drawtoken/task.hpp"

namespace rev {
	TLS<VideoParam> tls_videoParam;

	// ---------------- MainThread ----------------
	MainThread::MainThread():
		ThreadL("MainThread")
	{
		auto lk = _info.lock();
		lk->accumUpd = lk->accumDraw = 0;
		lk->tmBegin = Clock::now();
	}
	Timepoint MainThread::_WaitForNextInterval(const Timepoint prevtime, const Duration interval) {
		auto ntp = prevtime + interval;
		const auto tp = Clock::now();
		if(ntp <= tp)
			ntp = tp;
		else {
			const auto dur = ntp - tp;
			if(dur >= Microseconds(1000)) {
				// 時間に余裕があるならスリープをかける
				D_SDLWarn(SDL_Delay, std::chrono::duration_cast<Milliseconds>(dur).count() - 1);
			}
			// スピンウェイト
			while(Clock::now() < ntp);
		}
		return ntp;
	}
	namespace {
		// 描画スレッドがOpenGLコンテキストを初期化するまで待機し、MakeCurrentを呼ぶ(MultiContext時)
		void MakeCurrent(DrawThread& dth, const bool MC) {
			// ポーリングでコンテキスト初期化の完了を検知
			for(;;) {
				{
					auto op = dth.getInfo();
					// DrawThreadContextが初期化されていたら同時にMainThread用も初期化されている筈
					if(op->ctxDrawThread) {
						// ただしSingleContext環境ではNullなのでmakeCurrentしない
						if(MC) {
							auto lk = g_system_shared.lock();
							op->ctxMainThread->makeCurrent(lk->window.lock());
						}
						break;
					}
				}
				SDL_Delay(0);
			}
		}
		template <class T>
		using UPtr = std::unique_ptr<T>;
	}
	void MainThread::runL(const Looper_SP& guiLooper) {
		const bool MultiContext = g_system_shared.lock()->param->getMultiContext();
		UPtr<GLWrap>		glw;
		DrawThread			dth;
		Handler guiHandler(
					guiLooper,
					[](){
						// メッセージを受け取る度にSDLイベントを発生させる
						SDL_Event e;
						e.type = EVID_SIGNAL;
						SDL_PushEvent(&e);
					}
				);
		Handler drawHandler(Looper_WP{});
		try {
			glw = std::make_unique<GLWrap>(MultiContext);
			// 描画スレッドを先に初期化
			dth.start(std::ref(getLooper()));
			// 描画スレッドの初期化完了を待つ
			while(auto m = getLooper()->wait()) {
				if(static_cast<msg::DrawInit*>(*m))
					break;
			}
			// この時点でコンテキストが初期化出来てないとおかしい
			Assert0(dth.getInfo()->ctxDrawThread);
			GLW.initializeMainThread();
			drawHandler = Handler(dth.getLooper());
			{
				// 描画パラメータの初期化
				tls_videoParam = VideoParam{};
				auto& vp = *tls_videoParam;
				vp.bAnisotropic = false;
			}
			drawHandler.postExecNoWait([vp = *tls_videoParam](){
				tls_videoParam = vp;
			});

			MakeCurrent(dth, MultiContext);
			{
				// 各種リソースマネージャの初期化
				Manager mgr;
				FxReload rel;
				try {
					auto lk = g_system_shared.lock();
					_InitManagers(mgr, *lk->param);
				} catch (...) {
					// リソースマネージャの初期化に失敗した際は速やかにループを抜ける
					LogR(Error, "Exception thrown when initializing resource managers");
					throw;
				}
				{
					// 続いてメインスレッドインタフェースを初期化
					MainProc_UP mp;
					{
						auto lk = g_system_shared.lock();
						auto& param = lk->param;
						mp.reset(param->makeMainProc());
						// デフォルトエフェクトファイルを読み込み
						rel.curFx = param->makeEffect();
						lk->fx = rel.curFx;
						// 最初のシーンを作成
						mgr_scene.setPushScene(param->makeFirstScene(), false);
					}
					LogR(Verbose, "Mainproc initialized");
					guiHandler.postMessageNow(msg::MainInit());

					// シェーダーファイルが置いてあるディレクトリを監視
					FNotify ntf;
					_setupFxNotify(ntf);

					Timepoint prevtime = Clock::now();
					// ゲームの進行や更新タイミングを図って描画など
					bool bLoop = true;
					do {
						// 必要に応じてシェーダーファイルのリロード処理
						_checkFxReload(ntf, rel);

						if(!dth.isRunning()) {
							// 何らかの原因で描画スレッドが終了していた時
							try {
								// 例外が投げられて終了したかをチェック
								dth.getResult();
							} catch (...) {
								LogR(Verbose, "Draw thread was ended by throwing an exception");
								throw;
							}
							LogR(Verbose, "Draw thread was ended unexpectedly");
							break;
						}
						// 何かメッセージが来てたら処理する
						while(auto m = getLooper()->peek()) {
							if(static_cast<msg::PauseReq*>(*m)) {
								// ユーザーに通知(Pause)
								if(!mp->onPause()) {
									// 戻り値がfalseならゲーム進行を止めない
									continue;
								}
								// サウンド停止
								mgr_sound.pauseAllSound();
								// DrawThreadがIdleになるまで待つ
								while(dth.getInfo()->accum != getInfo()->accumDraw)
									SDL_Delay(0);

								std::stringstream buffer;	// backup/restore用一時バッファ
								bool bLoop = true;
								do {
									while(auto m = getLooper()->wait()) {
										if(static_cast<msg::ResumeReq*>(*m)) {
											// サウンド再開
											mgr_sound.resumeAllSound();
											// ユーザーに通知(Resume)
											mp->onResume();
											bLoop = false;
											break;
										} else if(static_cast<msg::StopReq*>(*m)) {
											// ユーザーに通知(Stop)
											mp->onStop();
											// 描画キューに入っているOpenGLリソースは無効になるのでここで削除
											mgr_drawtask.clear();
											// MultiContext環境ではContextの関連付けを解除
											if(MultiContext)
												dth.getInfo()->ctxMainThread->makeCurrent();
											// OpenGLリソースの解放をDrawThreadで行う
											drawHandler.postMessageNow(msg::DestroyContext());
											_Backup(mgr, buffer);
										} else if(static_cast<msg::ReStartReq*>(*m)) {
											_Restore(mgr, buffer);
											buffer.clear();
											buffer.str("");
											// DrawThreadでOpenGLの復帰処理を行う
											drawHandler.postMessageNow(msg::MakeContext());
											// 描画スレッドが確実にOpenGLの初期化を行うまで待つ
											drawHandler.postExec([](){});
											MakeCurrent(dth, MultiContext);
											// ユーザーに通知(Restart)
											mp->onReStart();
										}
									}
								} while(bLoop);
							}
						}
						// 次のフレーム開始時刻を待つ
						prevtime = _WaitForNextInterval(prevtime, Microseconds(16666));
						// 1フレーム分の処理を行う
						if(_updateFrame(mp.get(), dth, drawHandler))
							break;
					} while(bLoop);
					// シーンマネージャに積んであるシーンを全て終了させる
					while(auto scene = mgr_scene.getTop()) {
						mgr_scene.setPopScene(1);
						auto lk = g_system_shared.lock();
						const auto w = lk->window.lock();
						mgr_info.setInfo(w->getSize(), dth.getInfo()->fps.getFPS());
						mp->runU();
					}
				}
				GL.glFlush();
				// 描画スレッドの処理が追いつくまで待機
				LogR(Verbose, "Catch up Drawthread");
				drawHandler.postExec([](){});
			}
			// 描画スレッドを後片付けフェーズへ移行
			LogR(Verbose, "Requesting Drawthread to quit");
			drawHandler.postMessageNow(msg::QuitReq());
			// 後処理その1が終わるまで待つ
			while(dth.getInfo()->state != DrawThread::State::Ended)
				SDL_Delay(0);
			LogR(Verbose, "Exiting loop normally");
		} catch (const std::exception& e) {
			LogR(Error, "Exception thrown(%1%)", e.what());
		} catch (...) {
			LogR(Error, "Exception thrown(unknown exception)");
		}
		// MultiContext環境ではContextの関連付けを解除
		if(MultiContext)
			dth.getInfo()->ctxMainThread->makeCurrent();
		drawHandler.postMessageNow(msg::QuitReq());
		// 描画スレッドの終了を待つ
		dth.interrupt();
		dth.join();
		// GUIスレッドへ終了を伝える
		guiHandler.postMessageNow(msg::QuitReq());
		LogR(Verbose, "Ended");
	}
}
