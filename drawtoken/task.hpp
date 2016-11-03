#pragma once
#include "spine/singleton.hpp"
#include "handle.hpp"
#include "drawtoken.hpp"
#include "../sdl_mutex.hpp"

namespace rev {
	namespace draw {
		#define mgr_drawtask (::rev::draw::Task::ref())
		/*!
			PreFuncとして(TPStructR::applySettingsを追加)
			[Program, FrameBuff, RenderBuff]
		*/
		class Task : public spi::Singleton<Task> {
			public:
				constexpr static int NUM_TASK = 3;
			private:
				//! 描画エントリのリングバッファ
				draw::TokenML	_entry[NUM_TASK];
				HFx	 			_hFx[NUM_TASK];
				//! 読み書きカーソル位置
				int			_curWrite, _curRead;
				Mutex		_mutex;
				CondV		_cond;

			public:
				Task();
				draw::TokenML& refWriteEnt();
				draw::TokenML& refReadEnt();
				// -------------- from MainThread --------------
				void beginTask(HFx hFx);
				void endTask();
				void clear();
				// -------------- from DrawThread --------------
				void execTask();
		};
	}
}
