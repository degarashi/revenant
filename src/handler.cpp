#include "handler.hpp"
#include "looper.hpp"

namespace rev {
	// ----------------- Handler -----------------
	Handler::Handler(const Looper_WP& loop, const Callback& cb):
		_looper(loop),
		_cb(cb)
	{}
	void Handler::_callback() {
		if(_cb)
			_cb();
	}
	void Handler::handleMessage(const Message& msg) {
		if(msg.exec)
			msg.exec();
	}
	void Handler::post(Message&& m) {
		_callback();
		m.handler = m.exec ? this : nullptr;
		if(const auto sp = _looper.lock())
			sp->pushEvent(std::move(m));
	}
	const Looper_WP& Handler::getLooper() const noexcept {
		return _looper;
	}
	void Handler::postExec(const Exec& f) {
		CondV cond;
		Mutex mutex;
		UniLock lk(mutex);
		postMessageNow(msg::Exec(), [&](){
			f();
			UniLock lk2(mutex);
			cond.signal();
		});
		cond.wait(lk);
	}
	void Handler::postExecNoWait(const Exec& cb) {
		postMessageNow(msg::Exec(), cb);
	}
}
