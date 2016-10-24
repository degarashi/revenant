#pragma once
#include "sdl_tls.hpp"
#include "spine/optional.hpp"
#include <array>

namespace rev {
	//! SDLのMutexラッパ
	class Mutex {
		private:
			SDL_mutex*	_mutex;

		public:
			struct InitializeFailed : std::runtime_error {
				using std::runtime_error::runtime_error;
			};
			struct LockFailed : std::runtime_error {
				using std::runtime_error::runtime_error;
			};
			Mutex();
			Mutex(Mutex&& m) noexcept;
			Mutex(const Mutex&) = delete;
			Mutex& operator = (const Mutex&) = delete;
			~Mutex();

			void lock();
			bool try_lock();
			void unlock() noexcept;
			SDL_mutex* getMutex() noexcept;
	};
	class UniLock {
		private:
			Mutex*	_mutex;
			bool	_bLocked;

		public:
			static struct DeferLock_t {} DeferLock;
			static struct AdoptLock_t {} AdoptLock;
			static struct TryLock_t {} TryLock;

			UniLock() = delete;
			UniLock(const UniLock&) = delete;
			void operator = (const UniLock& u) = delete;
			UniLock(Mutex& m);
			UniLock(Mutex& m, DeferLock_t) noexcept;
			UniLock(Mutex& m, AdoptLock_t) noexcept;
			UniLock(Mutex& m, TryLock_t);
			~UniLock();
			UniLock(UniLock&& u);
			void lock();
			void unlock() noexcept;
			bool tryLock();
			bool isLocked() const noexcept;
			explicit operator bool () const noexcept;
			SDL_mutex* getMutex() noexcept;
	};
	class CondV {
		private:
			SDL_cond*	_cond;
		public:
			struct InitializeFailed : std::runtime_error {
				using std::runtime_error::runtime_error;
			};
			struct WaitFailed : std::runtime_error {
				using std::runtime_error::runtime_error;
			};
			struct SignalFailed : std::runtime_error {
				using std::runtime_error::runtime_error;
			};
			CondV();
			~CondV();
			void wait(UniLock& u);
			bool wait_for(UniLock& u, uint32_t msec);
			void signal();
			void signal_all();
	};
	namespace detail {
		struct CallUnlockR {
			template <class T>
			void operator()(T& t) const noexcept(noexcept(t._unlockR())) {
				return t._unlockR();
			}
		};
		struct CallUnlock {
			template <class T>
			void operator()(T& t) const noexcept(noexcept(t._unlock())) {
				return t._unlock();
			}
		};
		template <class SP, class T, class UnlockT>
		class SpinInner {
			private:
				SP&		_src;
				T*		_data;

			public:
				SpinInner(const SpinInner&) = delete;
				SpinInner& operator = (const SpinInner&) = delete;
				SpinInner(SpinInner&& n) noexcept:
					_src(n._src),
					_data(n._data)
				{
					n._data = nullptr;
				}
				SpinInner(SP& src, T* data) noexcept:
					_src(src),
					_data(data)
				{}
				~SpinInner() {
					unlock();
				}
				T& operator * () noexcept { return *_data; }
				T* operator -> () noexcept { return _data; }
				bool valid() const noexcept { return _data != nullptr; }
				explicit operator bool () const noexcept { return valid(); }
				void unlock() noexcept(noexcept(std::declval<UnlockT>()(_src))) {
					if(_data) {
						UnlockT()(_src);
						_data = nullptr;
					}
				}
				template <class T2>
				auto castAndMove() noexcept {
					SpinInner<SP, T2, UnlockT> ret(_src, reinterpret_cast<T2*>(_data));
					_data = nullptr;
					return ret;
				}
				template <class T2>
				auto castAndMoveDeRef() noexcept {
					SpinInner<SP, T2, UnlockT> ret(_src, reinterpret_cast<T2*>(*_data));
					_data = nullptr;
					return ret;
				}
		};
	}
	//! 内包データに対する再帰対応のスピンロック
	template <class T>
	class SpinLock {
		private:
			using Inner = detail::SpinInner<SpinLock<T>, T, detail::CallUnlock>;
			using CInner = detail::SpinInner<SpinLock<T>, const T, detail::CallUnlock>;
			friend struct detail::CallUnlock;

			SDL_atomic_t	_atmLock,
							_atmCount;
			void _unlock() noexcept {
				if(SDL_AtomicDecRef(&_atmCount) == SDL_TRUE)
					SDL_AtomicSet(&_atmLock, 0);
			}

			T	_data;
			template <class I>
			auto _lock(const bool bBlock) noexcept {
				do {
					bool bSuccess = false;
					if(SDL_AtomicCAS(&_atmLock, 0, *tls_threadID) == SDL_TRUE)
						bSuccess = true;
					else if(SDL_AtomicGet(&_atmLock) == static_cast<decltype(SDL_AtomicGet(&_atmLock))>(*tls_threadID)) {
						// 同じスレッドからのロック
						bSuccess = true;
					}

					if(bSuccess) {
						// ロック成功
						SDL_AtomicAdd(&_atmCount, 1);
						return I(*this, &_data);
					}
				} while(bBlock);
				return I(*this, nullptr);
			}

		public:
			SpinLock() noexcept {
				SDL_AtomicSet(&_atmLock, 0);
				SDL_AtomicSet(&_atmCount, 0);
			}
			Inner lock() noexcept {
				return _lock<Inner>(true);
			}
			CInner lockC() const noexcept {
				return const_cast<SpinLock*>(this)->_lock<CInner>(true);
			}
			Inner try_lock() noexcept {
				return _lock<Inner>(false);
			}
			CInner try_lockC() const noexcept {
				return const_cast<SpinLock*>(this)->_lock<CInner>(false);
			}
	};
	//! 一時的なロック解除機能を備えたSpinLock
	/*! あるスレッドがロック中でも一旦アンロックし、別のスレッドがロックできるようにする */
	template <class T>
	class SpinLockPut {
		private:
			struct InnerP {
				SpinLockPut&	_s;
				bool		_bLocked;
				InnerP(InnerP&& p) noexcept:
					_s(p._s),
					_bLocked(p._bLocked)
				{
					p._bLocked = false;
				}
				InnerP(SpinLockPut& s):
					_s(s)
				{
					_bLocked = _s._put();
				}
				~InnerP() noexcept(false) {
					if(_bLocked)
						_s._put_reset();
				}
			};
			using Inner = detail::SpinInner<SpinLockPut<T>, T, detail::CallUnlock>;
			using CInner = detail::SpinInner<SpinLockPut<T>, const T, detail::CallUnlock>;
			friend struct detail::CallUnlock;
			using ThreadID_OP = spi::Optional<SDL_threadID>;
		
			TLS<int>		_tlsCount;
			ThreadID_OP		_lockID;
			int				_lockCount;
			Mutex			_mutex;
			T				_data;

			void _unlock() NOEXCEPT_IF_RELEASE {
				D_Assert0(_lockID && *_lockID == *tls_threadID);
				D_Assert0(_lockCount >= 1);
				if(--_lockCount == 0)
					_lockID = spi::none;
				_mutex.unlock();
			}
			template <class I>
			I _lock(bool bBlock) {
				if(bBlock)
					_mutex.lock();
				if(bBlock || _mutex.try_lock()) {
					if(!_lockID) {
						_lockCount = 1;
						_lockID = *tls_threadID;
					} else {
						D_Assert0(*_lockID == *tls_threadID);
						++_lockCount;
					}
					return I(*this, &_data);
				}
				return I(*this, nullptr);
			}
			void _put_reset() {
				_mutex.lock();
				// TLS変数に対比してた回数分、再度MutexのLock関数を呼ぶ
				_lockID = *tls_threadID;
				_lockCount = _tlsCount.get()-1;
				*_tlsCount = -1;

				int tmp = _lockCount;
				while(--tmp != 0)
					_mutex.lock();
			}
			bool _put() {
				// 自スレッドがロックしてたらカウンタを退避して一旦解放
				if(_mutex.try_lock()) {
					if(_lockCount > 0) {
						++_lockCount;
						*_tlsCount = _lockCount;	// ロックしてた回数をTLS変数に退避
						int tmp = _lockCount;
						_lockCount = 0;
						_lockID = spi::none;
						// 今までロックしてた回数分、MutexのUnlock回数を呼ぶ
						while(tmp-- != 0)
							_mutex.unlock();
						return true;
					}
					_mutex.unlock();
				}
				return false;
			}

		public:
			SpinLockPut():
				_lockCount(0)
			{
				_tlsCount = -1;
			}
			Inner lock() {
				return _lock<Inner>(true);
			}
			CInner lockC() const {
				return const_cast<SpinLockPut*>(this)->_lock<CInner>(true);
			}
			Inner try_lock() {
				return _lock<Inner>(false);
			}
			CInner try_lockC() const {
				return const_cast<SpinLockPut*>(this)->_lock<CInner>(false);
			}
			InnerP put() {
				return InnerP(*this);
			}
	};
	//! 読み込みロックは複数で同時に出来るが書き込みは排他
	template <class T, int NRead>
	class SpinLockRW {
		private:
			Mutex			_mutex;
	
			using InnerW = detail::SpinInner<SpinLockRW<T,NRead>, T, detail::CallUnlock>;
			using InnerR = detail::SpinInner<SpinLockRW<T,NRead>, const T, detail::CallUnlockR>;
			friend struct detail::CallUnlock;
			friend struct detail::CallUnlockR;
			using ThreadID = SDL_threadID;
			struct ThreadEnt {
				ThreadID	threadId;
				int			count;

				ThreadEnt(ThreadID id) noexcept:
					threadId(id),
					count(1)
				{}
				void increment() noexcept {
					++count;
				}
				bool decrement() noexcept {
					return --count == 0;
				}
			};
			using ThreadEnt_OP = spi::Optional<ThreadEnt>;
			std::array<ThreadEnt_OP, NRead>	_read;
			ThreadEnt_OP					_write;
			T								_data;

			auto _lockMutex() {
				return UniLock(_mutex, UniLock::TryLock);
			}
			void _unlock() {
				// Writeロックがされているという事は他に誰もReadロックしてない
				for(auto& e : _read) {
					D_Assert0(!e || e->threadId==*tls_threadID);
				}
				for(;;) {
					if(auto lk = _lockMutex()) {
						if(_write->decrement())
							_write = spi::none;
						return;
					}
				}
			}
			void _unlockR() {
				// Readロックがされているという事はWriteロックはされてない
				D_Assert0(!_write || _write->threadId==*tls_threadID);
				for(;;) {
					if(auto lk = _lockMutex()) {
						for(auto& e : _read) {
							if(e && e->threadId == *tls_threadID) {
								if(e->decrement())
									e = spi::none;
								return;
							}
						}
						D_Assert0(false);
						return;
					}
				}
			}
			bool _findSetReadEnt() noexcept {
				spi::Optional<int> emptyIndex;
				// ReadThreadIdスロットに自分のIdがあるか
				for(int i=0 ; i<NRead ; i++) {
					auto& e = _read[i];
					// ついでに空きスロットを見つけたら記録しておく
					if(!e)
						emptyIndex = i;
					else if(e->threadId == *tls_threadID) {
						e->increment();
						return true;
					}
				}
				// ReadThreadIdスロットに空きがあるか
				if(emptyIndex) {
					// ThreadIdを登録
					_read[*emptyIndex] = ThreadEnt(*tls_threadID);
					return true;
				}
				return false;
			}
			InnerR _lockR(const bool bBlock) {
				do {
					if(auto lk = _lockMutex()) {
						// 他のスレッドにWriteロックされていたらReadロックできない
						if(!_write || _write->threadId == *tls_threadID) {
							if(_findSetReadEnt())
								return InnerR(*this, &_data);
							D_Assert0(!_write || _write->threadId != *tls_threadID);
						}
					}
				} while(bBlock);
				// ロック失敗
				return InnerR(*this, nullptr);
			}
			InnerW _lock(const bool bBlock) {
				do {
					if(auto lk = _lockMutex()) {
						// ReadロックがされてたらWriteロックできない
						bool hasReadLock(false);
						for(auto& e : _read) {
							hasReadLock |= (e && e->threadId != *tls_threadID);
						}
						if(!hasReadLock) {
							// Writeがロックされているか
							if(!_write) {
								// 自分のIDを書き込む
								_write = {*tls_threadID};
								return InnerW(*this, &_data);
							} else if(_write->threadId == *tls_threadID) {
								// Write再帰ロック
								_write->increment();
								return InnerW(*this, &_data);
							}
							else {
								// ロック失敗
							}
						}
					}
				} while(bBlock);
				// ロック失敗
				return InnerW(*this, nullptr);
			}
		public:
			SpinLockRW() {
				static_assert(NRead > 0, "invalid NRead value");
			}
			auto readLock() { return _lockR(true); }
			auto try_readLock() { return _lockR(false); }
			auto writeLock() { return _lock(true); }
			auto try_writeLock() { return _lock(false); }
	};
}
