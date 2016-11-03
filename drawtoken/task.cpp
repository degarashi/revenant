#include "task.hpp"
#include "../gl_if.hpp"

namespace rev {
	namespace draw {
		// -------------- Task --------------
		Task::Task():
			_curWrite(0),
			_curRead(0)
		{}
		TokenML& Task::refWriteEnt() {
			UniLock lk(_mutex);
			return _entry[_curWrite % NUM_TASK];
		}
		TokenML& Task::refReadEnt() {
			UniLock lk(_mutex);
			return _entry[_curRead % NUM_TASK];
		}
		void Task::beginTask(HFx hFx) {
			UniLock lk(_mutex);
			// 読み込みカーソルが追いついてない時は追いつくまで待つ
			auto diff = _curWrite - _curRead;
			Assert0(diff >= 0);
			while(diff >= NUM_TASK) {
				_cond.wait(lk);
				diff = _curWrite - _curRead;
				Assert0(diff >= 0);
			}
			auto& we = refWriteEnt();
			lk.unlock();
			_hFx[_curWrite % NUM_TASK] = hFx;
			we.clear();
		}
		void Task::endTask() {
			GL.glFlush();
			UniLock lk(_mutex);
			++_curWrite;
		}
		void Task::clear() {
			UniLock lk(_mutex);
			_curWrite = _curRead+1;
			GL.glFinish();
			for(auto& e : _entry)
				e.clear();
			for(auto& h : _hFx)
				h.reset();
			_curWrite = _curRead = 0;
		}
		void Task::execTask() {
			spi::Optional<UniLock> lk(_mutex);
			auto diff = _curWrite - _curRead;
			Assert0(diff >= 0);
			if(diff > 0) {
				auto& readent = refReadEnt();
				lk = spi::none;
				// MThとアクセスするエントリが違うから同期をとらなくて良い
				readent.exec();
				GL.glFlush();
				lk = spi::construct(_mutex);
				++_curRead;
				_cond.signal();
			}
		}
	}
}
