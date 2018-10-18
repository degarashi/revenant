#pragma once
#include "spinlock_detail.hpp"
#include <array>

namespace rev {
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
