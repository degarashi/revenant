#pragma once
#include "tokenml.hpp"
#include "../sdl_mutex.hpp"

namespace rev {
	namespace draw {
		/*!
			PreFuncとして(TPStructR::applySettingsを追加)
			[Program, FrameBuff, RenderBuff]
		*/
		class Task {
			public:
				constexpr static int NUM_TASK = 3;
			private:
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
