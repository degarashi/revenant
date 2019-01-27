#pragma once
#include "spinlock_detail.hpp"
#include "sdl_tls.hpp"

namespace rev {
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
}
