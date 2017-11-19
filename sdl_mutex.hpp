#pragma once
#include "sdl_tls.hpp"
#include "spine/optional.hpp"

namespace rev {
	//! SDLのMutexラッパ
	class Mutex {
		private:
			SDL_mutex*	_mutex;

		public:
			// ---- SDLに起因するエラー時に投げられる例外クラス ----
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
	//! UniqueLock: コンストラクタでロック、デストラクタでリリース
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
	//! ConditionalValue: ラッチ等の実装に使う
	class CondV {
		private:
			SDL_cond*	_cond;
		public:
			// ---- SDLに起因するエラー時に投げられる例外クラス ----
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
}
