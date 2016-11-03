#pragma once
#include "looper.hpp"
#include "sdl_thread.hpp"

namespace rev {
	//! Looper付きスレッド
	template <class T>
	class ThreadL;
	template <class RET, class... Args>
	class ThreadL<RET (Args...)> : public Thread<RET (Args...)> {
		private:
			using base = Thread<RET (Args...)>;
			Looper_SP	_spLooper;
		protected:
			using base::base;
			RET run(Args... args) override final {
				Looper::Prepare();
				_spLooper = Looper::GetLooper();
				return runL(std::forward<Args>(args)...);
			}
			virtual RET runL(Args... args) = 0;
		public:
			bool interrupt() override {
				if(base::interrupt()) {
					_spLooper->setState(false);
					return true;
				}
				return false;
			}
			const Looper_SP& getLooper() {
				return _spLooper;
			}
	};
}
