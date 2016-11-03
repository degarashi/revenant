#include "looper.hpp"
#include "handler.hpp"

namespace rev {
	// ------------- MsgId -------------
	bool MsgId::operator == (const MsgId& d) const noexcept {
		return id == d.id;
	}
	bool MsgId::operator < (const MsgId& d) const noexcept {
		return id < d.id;
	}
	MsgId::operator int () const noexcept {
		return id;
	}

	int GetNewMessageId() {
		static int s_messageID = 0;
		return s_messageID++;
	}

	// ------------- Message -------------
	const Duration Message::NoDelay = Seconds(0);
	Message::Message(const Duration delay, Exec&& e) noexcept:
		tpoint(Clock::now() + delay), id{-1},
		handler(nullptr),
		exec(std::move(e))
	{}
	bool Message::operator < (const Message& m) const noexcept {
		return tpoint < m.tpoint;
	}

	// ----------------- Looper -----------------
	TLS<Looper_SP> Looper::tls_looper;
	Looper::Looper(Looper&& lp) noexcept:
		_msg(std::move(lp._msg))
	{}
	Looper& Looper::operator = (Looper&& lp) noexcept {
		std::swap(_msg, lp._msg);
		return *this;
	}
	void Looper::Prepare() {
		Assert0(!tls_looper);
		tls_looper = Looper_SP(new Looper());
	}
	Looper::Message_OP Looper::_procMessage(std::function<bool (UniLock&)> lf) {
		UniLock lk(_mutex);
		auto tpoint = Clock::now();
		while(_bRun) {
			if(_msg.empty() ||
				tpoint < _msg.front().tpoint)
			{
				if(!lf(lk))
					break;
				tpoint = Clock::now();
			} else {
				Message m;
				_msg.pop_front(m);
				if(m.handler) {
					// ハンドラ持ちのメッセージはその場で実行
					m.handler->handleMessage(m);
				} else {
					// ユーザーに返して処理してもらう
					return std::move(m);
				}
			}
		}
		return spi::none;
	}
	Looper::Message_OP Looper::wait() {
		return _procMessage([this](UniLock& lk) {
			if(_msg.empty())
				_cond.wait(lk);
			else {
				auto dur = _msg.front().tpoint - Clock::now();
				std::chrono::milliseconds msec(std::chrono::duration_cast<std::chrono::milliseconds>(dur));
				_cond.wait_for(lk, std::max<std::chrono::milliseconds::rep>(0, msec.count()));
			}
			return true;
		});
	}
	Looper::Message_OP Looper::peek(const Milliseconds msec) {
		return _procMessage([this, msec](UniLock& lk) {
			return _cond.wait_for(lk, msec.count());
		});
	}
	void Looper::pushEvent(Message&& m) {
		UniLock lk(_mutex);
		_msg.push(std::move(m));
		_cond.signal_all();
	}
	const Looper_SP& Looper::GetLooper() {
		return *tls_looper;
	}
	void Looper::setState(const bool bRun) {
		UniLock lk(_mutex);
		_bRun = bRun;
		if(!bRun)
			_cond.signal_all();
	}
}
