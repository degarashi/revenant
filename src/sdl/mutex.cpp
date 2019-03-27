#include "mutex.hpp"
#include "error.hpp"

namespace rev {
	// ----- Mutex -----
	Mutex::Mutex():
		_mutex(SDL_CreateMutex())
	{
		if(!_mutex)
			throw InitializeFailed("Mutex::Mutex()");
	}
	Mutex::Mutex(Mutex&& m) noexcept:
		_mutex(m._mutex)
	{
		m._mutex = nullptr;
	}
	Mutex::~Mutex() {
		if(_mutex)
			D_SDLWarn(SDL_DestroyMutex, _mutex);
	}
	void Mutex::lock() {
		const auto res = D_SDLAssert(SDL_LockMutex, _mutex);
		if(res < 0)
			throw LockFailed("Mutex::lock()");
	}
	bool Mutex::try_lock() {
		// 単に他がロックしている時もTryLockが失敗するのでSDL_GetErrorによるエラーチェックはしない
		const auto res = SDL_TryLockMutex(_mutex);
		if(res == 0)
			return true;
		if(res == SDL_MUTEX_TIMEDOUT) {
			// TryLockが失敗した時に後のエラーチェックに響くため、ここでリセット
			SDL_ClearError();
			return false;
		}
		D_SDLWarn0();
		throw LockFailed("Mutex::try_lock()");
	}
	void Mutex::unlock() noexcept {
		D_SDLWarn(SDL_UnlockMutex, _mutex);
	}
	SDL_mutex* Mutex::getMutex() noexcept {
		return _mutex;
	}

	// ----- UniLock -----
	UniLock::UniLock(Mutex& m):
		_mutex(&m),
		_bLocked(true)
	{
		m.lock();
	}
	UniLock::UniLock(Mutex& m, DeferLock_t) noexcept:
		_mutex(&m),
		_bLocked(false)
	{}
	UniLock::UniLock(Mutex& m, AdoptLock_t) noexcept:
		_mutex(&m),
		_bLocked(true)
	{}
	UniLock::UniLock(Mutex& m, TryLock_t):
		_mutex(&m),
		_bLocked(m.try_lock())
	{}
	UniLock::~UniLock() {
		unlock();
	}
	UniLock::UniLock(UniLock&& u):
		_mutex(u._mutex),
		_bLocked(u._bLocked)
	{
		u._mutex = nullptr;
		u._bLocked = false;
	}
	void UniLock::lock() {
		if(!_bLocked) {
			_mutex->lock();
			_bLocked = true;
		}
	}
	bool UniLock::tryLock() {
		if(!_bLocked)
			return _bLocked = _mutex->try_lock();
		return true;
	}
	void UniLock::unlock() noexcept {
		if(_bLocked) {
			_mutex->unlock();
			_bLocked = false;
		}
	}
	bool UniLock::isLocked() const noexcept {
		return _bLocked;
	}
	SDL_mutex* UniLock::getMutex() noexcept {
		if(_mutex)
			return _mutex->getMutex();
		return nullptr;
	}
	UniLock::operator bool() const noexcept {
		return isLocked();
	}

	// ----- CondV -----
	CondV::CondV():
		_cond(SDL_CreateCond())
	{
		if(!_cond)
			throw InitializeFailed("CondV::CondV()");
	}
	CondV::~CondV() {
		D_SDLWarn(SDL_DestroyCond, _cond);
	}
	void CondV::wait(UniLock& u) {
		D_Assert0(u.isLocked());
		const auto res = D_SDLAssert(SDL_CondWait, _cond, u.getMutex());
		if(res < 0)
			throw WaitFailed("CondV::wait()");
	}
	bool CondV::wait_for(UniLock& u, const uint32_t msec) {
		D_Assert0(u.isLocked());
		D_Expect0(msec > 0);
		const auto res = D_SDLAssert(SDL_CondWaitTimeout, _cond, u.getMutex(), std::max<decltype(msec)>(1, msec));
		if(res == 0)
			return true;
		if(res == SDL_MUTEX_TIMEDOUT)
			return false;
		throw WaitFailed("CondV::wait_for()");
	}
	void CondV::signal() {
		if(D_SDLAssert(SDL_CondSignal, _cond) != 0)
			throw SignalFailed("CondV::signal()");
	}
	void CondV::signal_all() {
		if(D_SDLAssert(SDL_CondBroadcast, _cond) != 0)
			throw SignalFailed("CondV::signal_all()");
	}
}
