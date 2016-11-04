#pragma once
#include <functional>
#include <memory>
#include "clock.hpp"

namespace rev {
	struct Message;
	using Exec = std::function<void ()>;
	class Looper;
	using Looper_WP = std::weak_ptr<Looper>;
	class Handler {
		private:
			using Callback = std::function<void ()>;
			Looper_WP	_looper;
			Callback	_cb;

			friend class Looper;
			void _callback();
		protected:
			virtual void handleMessage(const Message& msg);
		public:
			Handler(Handler&& h) = default;
			Handler(const Looper_WP& loop, Callback cb=Callback());
			Handler& operator = (Handler&&) = default;
			virtual ~Handler() {}
			const Looper_WP& getLooper() const noexcept;
			//! コールバック関数を渡し、それの実行が終わるまで待機
			void postExec(const Exec& f);
			//! コールバック関数を渡し、実行完了を待たずに戻る
			void postExecNoWait(Exec f);
			void post(Message&& m);
			template <class... Args>
			void postArgsDelay(Args&&... args) {
				_callback();
				post(Message(std::forward<Args>(args)...));
			}
			template <class... Args>
			void postArgs(Args&&... args) {
				_callback();
				post(Message(rev::Seconds(0), std::forward<Args>(args)...));
			}
	};
}
