#pragma once
#include <SDL_thread.h>
#include "mutex.hpp"
#include "spine/src/optional.hpp"
#include "spine/src/argholder.hpp"
#include "spine/src/enum.hpp"

namespace rev {
	DefineEnum(ThreadState,
		(Idle)					//< スレッド開始前
		(Running)				//< スレッド実行中
		(Interrupted)			//< 中断要求がされたが、まだスレッドが終了していない
		(Interrupted_End)		//< 中断要求によりスレッドが終了した
		(Error_End)				//< 例外による異常終了
		(Finished)				//< 正常終了
	);
	template <class DER, class RET, class... Args>
	class _Thread {
		private:
			using This = _Thread<DER, RET, Args...>;
			SDL_atomic_t		_atmInt;
			SDL_Thread*			_thread;

			using Holder = spi::ArgHolder<Args...>;
			using Holder_OP = spi::Optional<Holder>;
			Holder_OP			_holder;

			SDL_atomic_t		_atmStat;
			SDL_cond			*_condC,		//!< 子スレッドが開始された事を示す
								*_condP;		//!< 親スレッドがクラス変数にスレッドポインタを格納した事を示す
			Mutex				_mtxC,
								_mtxP;
			std::string			_name;

		static int ThreadFunc(void* p) {
			DER* ths = reinterpret_cast<DER*>(p);
			ths->_mtxC.lock();
			ths->_mtxP.lock();
			SDL_CondBroadcast(ths->_condC);
			ths->_mtxC.unlock();
			// 親スレッドが子スレッド変数をセットするまで待つ
			SDL_CondWait(ths->_condP, ths->_mtxP.getMutex());
			ths->_mtxP.unlock();

			// この時点でスレッドのポインタは有効な筈
			tls_threadID = SDL_GetThreadID(ths->_thread);
			tls_threadName = ths->_name;
			ThreadState stat;
			try {
				ths->_holder->inorder([ths](Args... args){ ths->runIt(std::forward<Args>(args)...); });
				stat = (ths->isInterrupted()) ? ThreadState::Interrupted_End : ThreadState::Finished;
			} catch(...) {
				Expect(false, "thread is finished unexpectedly");
				#ifdef NO_EXCEPTION_PTR
					ths->_bException = true;
				#else
					ths->_eptr = std::current_exception();
				#endif
				stat = ThreadState::Error_End;
			}
			SDL_AtomicSet(&ths->_atmStat, stat);
			SDL_CondBroadcast(ths->_condP);
			// SDLの戻り値は使わない
			return 0;
		}
			//! move-ctorの後に値を掃除する
			void _clean() {
				_thread = nullptr;
				_holder = spi::none;
				_condC = _condP = nullptr;
			}
		protected:
			#ifdef NO_EXCEPTION_PTR
				bool	_bException = false;
			#else
				std::exception_ptr	_eptr = nullptr;
			#endif
			virtual RET run(Args...) = 0;
		public:
			_Thread(_Thread&& th):
				_atmInt(std::move(th._atmInt)),
				_thread(th._thread),
				_holder(std::move(th._holder)),
				_atmStat(std::move(th._atmStat)),
				_condC(th._condC),
				_condP(th._condP),
				_mtxC(std::move(th._mtxC)),
				_mtxP(std::move(th._mtxP)),
				_name(std::move(th._name))
			{
				th._clean();
			}
			_Thread(const _Thread& t) = delete;
			_Thread& operator = (const _Thread& t) = delete;
			_Thread(const std::string& name):
				_thread(nullptr),
				_name(name)
			{
				_condC = SDL_CreateCond();
				_condP = SDL_CreateCond();
				// 中断フラグに0をセット
				SDL_AtomicSet(&_atmInt, 0);
				SDL_AtomicSet(&_atmStat, ThreadState::Idle);
			}
			~_Thread() {
				if(_thread) {
					// スレッド実行中だったらエラーを出す
					Assert0(!isRunning());
					SDL_DestroyCond(_condC);
					SDL_DestroyCond(_condP);
				}
			}
			template <class... Args0>
			void start(Args0&&... args0) {
				_holder = Holder(std::forward<Args0>(args0)...);
				// 2回以上呼ぶとエラー
				Assert0(SDL_AtomicGet(&_atmStat) == ThreadState::Idle);
				SDL_AtomicSet(&_atmStat, ThreadState::Running);

				_mtxC.lock();
				// 一旦クラス内部に変数を参照で取っておく
				SDL_Thread* th = SDL_CreateThread(ThreadFunc, _name.c_str(), this);
				// 子スレッドが開始されるまで待つ
				SDL_CondWait(_condC, _mtxC.getMutex());
				_mtxC.unlock();

				_mtxP.lock();
				_thread = th;
				SDL_CondBroadcast(_condP);
				_mtxP.unlock();
			}
			ThreadState getStatus() noexcept {
				return static_cast<ThreadState::e>(SDL_AtomicGet(&_atmStat));
			}
			bool isRunning() noexcept {
				const ThreadState stat = getStatus();
				return stat==ThreadState::Running || stat==ThreadState::Interrupted;
			}
			//! 中断を指示する
			virtual bool interrupt() noexcept {
				auto ret = SDL_AtomicCAS(&_atmStat, ThreadState::Running, ThreadState::Interrupted);
				SDL_AtomicSet(&_atmInt, 1);
				return ret == SDL_TRUE;
			}
			//! スレッド内部で中断指示がされているかを確認
			bool isInterrupted() noexcept {
				return SDL_AtomicGet(&_atmInt) == 1;
			}
			void join() {
				Assert0(getStatus() != ThreadState::Idle);
				_mtxP.lock();
				if(_thread) {
					_mtxP.unlock();
					SDL_WaitThread(_thread, nullptr);
					_thread = nullptr;
					SDL_DestroyCond(_condP);
					SDL_DestroyCond(_condC);
					_condP = _condC = nullptr;
				} else
					_mtxP.unlock();
			}
			bool try_join(const uint32_t ms) {
				Assert0(getStatus() != ThreadState::Idle);
				_mtxP.lock();
				if(_thread) {
					const int res = SDL_CondWaitTimeout(_condP, _mtxP.getMutex(), ms);
					_mtxP.unlock();
					SDLAssert0();
					if(res == 0 || !isRunning()) {
						SDL_WaitThread(_thread, nullptr);
						_thread = nullptr;
						return true;
					}
					return false;
				}
				_mtxP.unlock();
				return true;
			}
			void getResult() {
				// まだスレッドが終了して無い時の為にjoinを呼ぶ
				join();
				rethrowIfException();
			}
			void rethrowIfException() {
				#ifdef NO_EXCEPTION_PTR
					if(_bException)
						throw std::runtime_error("exception catched in thread");
				#else
					if(_eptr)
						std::rethrow_exception(_eptr);
				#endif
			}
			const std::string& getName() const noexcept {
				return _name;
			}
	};
	/*!
		\tparam SIG		スレッドに関するシグニチャ
	*/
	template <class SIG>
	class Thread;
	template <class RET, class... Args>
	class Thread<RET (Args...)> : public _Thread<Thread<RET (Args...)>, RET, Args...> {
		private:
			using base_type = _Thread<Thread<RET (Args...)>, RET, Args...>;
			spi::Optional<RET> 	_retVal;
		protected:
			virtual RET run(Args...) = 0;
		public:
			using base_type::base_type;
			void runIt(Args... args) {
				_retVal = run(std::forward<Args>(args)...);
			}
			RET&& getResult() {
				// まだスレッドが終了して無い時の為にjoinを呼ぶ
				base_type::join();
				base_type::rethrowIfException();
				return std::move(*_retVal);
			}
	};
	template <class... Args>
	class Thread<void (Args...)> : public _Thread<Thread<void (Args...)>, void, Args...> {
		private:
			using base_type = _Thread<Thread<void (Args...)>, void, Args...>;
		protected:
			virtual void run(Args...) = 0;
		public:
			using base_type::base_type;
			void runIt(Args... args) {
				run(std::forward<Args>(args)...);
			}
			void getResult() {
				// まだスレッドが終了して無い時の為にjoinを呼ぶ
				base_type::join();
				base_type::rethrowIfException();
			}
	};
}
