#pragma once
#include "tokenml.hpp"
#include "../sdl_mutex.hpp"

namespace rev {
	namespace draw {
		class Task {
			private:
				constexpr static int NUM_TASK = 3;
				//! 描画エントリのリングバッファ
				draw::TokenML	_entry[NUM_TASK];
				//! 読み書きカーソル位置
				int			_curWrite, _curRead;
				Mutex		_mutex;
				CondV		_cond;

			public:
				Task();
				draw::TokenML& refWriteEnt();
				draw::TokenML& refReadEnt();
				// -------------- from MainThread --------------
				void beginTask();
				void endTask();
				void clear();
				// -------------- from DrawThread --------------
				void execTask();
		};
	}
}
