#pragma once
#include "spine/singleton.hpp"
#include "spine/enum.hpp"
#include "looper_thread.hpp"
#include "fpscounter.hpp"
#include "drawproc.hpp"
#include "spinlock.hpp"
#include <memory>

namespace rev {
	class GLContext;
	using GLContext_SP = std::shared_ptr<GLContext>;
	//! 描画スレッド
	class DrawThread : public spi::Singleton<DrawThread>,
						public ThreadL<void (const Looper_SP&)>
	{
		public:
			DefineEnum(
				State,
				(Idle)			// 次のタスク待ち
				(Drawing)		// 描画処理中
				(Ended)			// 終了処理完了
			);
		private:
			struct Info {
				State			state = State::Idle;	//!< 現在の動作状態(描画中か否か)
				uint64_t		accum = 0;				//!< 描画が終わったフレーム番号
				// SingleContext環境ではctxMainThread = nullptr
				GLContext_SP	ctxDrawThread,			//!< 描画スレッド用のGLコンテキスト
								ctxMainThread;			//!< メインスレッド用のGLコンテキスト
				FPSCounter		fps;
			};
			SpinLock<Info>		_info;
			DrawProc_UP			_dproc;
		protected:
			void runL(const Looper_SP& mainLooper) override;
		public:
			DrawThread();
			decltype(auto) getInfo() { return _info.lock(); }
			decltype(auto) getInfo() const { return _info.lockC(); }
	};
}
