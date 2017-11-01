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
	Message::Message(const Duration delay, const Exec& e) noexcept:
		tpoint(Clock::now() + delay), id{-1},
		handler(nullptr),
		exec(e)
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
		if(!tls_looper)
			tls_looper = Looper_SP(new Looper());
	}
	Looper::Message_OP Looper::_procMessage(std::function<bool (UniLock&)> lf) {
		UniLock lk(_mutex);
		auto tpoint = Clock::now();
		while(_bRun) {
			// メッセージキューが空になるか、まだ処理すべき時間でないなら終了
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
				// 次のメッセージが配信される時間までsleep (or 途中でメッセージが来ればwakeup)
				const auto dur = _msg.front().tpoint - Clock::now();
				const Milliseconds msec(std::chrono::duration_cast<Milliseconds>(dur));
				_cond.wait_for(lk, std::max<Milliseconds::rep>(0, msec.count()));
			}
			return true;
		});
	}
	Looper::Message_OP Looper::peek() {
		return _procMessage([](UniLock&) {
			return false;
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
