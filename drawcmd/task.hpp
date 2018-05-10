#pragma once
#include "../sdl_mutex.hpp"
#include "cmd.hpp"

namespace rev::draw {
	class Task {
		private:
			constexpr static std::size_t NumTask = 3;
			//! 描画エントリのリングバッファ
			//! 読み書きカーソル位置
			CommandVec	_cmd[NumTask];
			struct {
				std::size_t	write,
							read;
			} _cursor;
			Mutex		_mutex;
			CondV		_cond;

			CommandVec& _writeEnt();
			CommandVec& _readEnt();
		public:
			Task();
			// -------------- from MainThread --------------
			IQueue& beginTask();
			void endTask();
			void clear();
			// -------------- from DrawThread --------------
			void execTask();
	};
}
