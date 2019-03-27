#include "task.hpp"
#include "../gl/if.hpp"

namespace rev::draw {
	Task::Task() {
		_cursor.write = _cursor.read = 0;
	}
	CommandVec& Task::_writeEnt() {
		UniLock lk(_mutex);
		return _cmd[_cursor.write % NumTask];
	}
	CommandVec& Task::_readEnt() {
		UniLock lk(_mutex);
		return _cmd[_cursor.read % NumTask];
	}
	IQueue& Task::beginTask() {
		UniLock lk(_mutex);
		// 読み込みカーソルが追いついてない時は追いつくまで待つ
		auto diff = _cursor.write - _cursor.read;
		Assert0(diff >= 0);
		while(diff >= NumTask) {
			_cond.wait(lk);
			diff = _cursor.write - _cursor.read;
			Assert0(diff >= 0);
		}
		auto& we = _writeEnt();
		lk.unlock();
		we.clear();
		return we;
	}
	void Task::endTask() {
		GL.glFlush();
		UniLock lk(_mutex);
		++_cursor.write;
	}
	void Task::clear() {
		UniLock lk(_mutex);
		_cursor.write = _cursor.read+1;
		GL.glFinish();
		for(auto& c : _cmd)
			c.clear();
		_cursor.write = _cursor.read = 0;
	}
	void Task::execTask() {
		spi::Optional<UniLock> lk(_mutex);
		auto diff = _cursor.write - _cursor.read;
		Assert0(diff >= 0);
		if(diff > 0) {
			auto& readent = _readEnt();
			lk = spi::none;
			// MThとアクセスするエントリが違うから同期をとらなくて良い
			readent.exec();
			GL.glFlush();
			lk = spi::construct(_mutex);
			++_cursor.read;
			_cond.signal();
		}
	}
}
