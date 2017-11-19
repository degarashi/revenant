#pragma once
#include "spinlock_detail.hpp"

namespace rev {
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
}
