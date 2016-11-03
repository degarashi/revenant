#pragma once
#include "clock.hpp"
#include "lubee/meta/enable_if.hpp"
#include "spine/pqueue.hpp"
#include "sdl_tls.hpp"
#include "sdl_mutex.hpp"
#include <functional>
#include <cstring>
#include <memory>

namespace rev {
	struct MsgId {
		int		id;
		operator int () const noexcept;
		bool operator == (const MsgId& m) const noexcept;
		bool operator < (const MsgId& m) const noexcept;
	};

	int GetNewMessageId();
	template <class T>
	struct MsgBase {
		const static MsgId Id;
	};
	template <class T>
	const MsgId MsgBase<T>::Id{ GetNewMessageId() };

	namespace msg {
		struct Exec : MsgBase<Exec> {};
	}

	using Exec = std::function<void ()>;
	class Handler;
	struct Message {
		const static Duration NoDelay;
		Timepoint	tpoint;
		MsgId		id;
		// IExecを呼び出すまでもないパラメータを渡す時に使用
		struct Data {
			constexpr static int PayloadSize = 32;
			uint8_t		data[PayloadSize];
			Data() = default;
			Data(const Data& d) {
				std::memcpy(this, &d, sizeof(*this));
			}
		};
		Data		data;
		Handler*	handler;
		Exec		exec;

		Message() = default;
		Message(const Message& m) = delete;
		Message(Message&& m) = default;
		Message(Duration delay, Exec&& e) noexcept;
		template <
			class T,
			ENABLE_IF((std::is_base_of<MsgBase<T>,T>{}))
		>
		Message(const Duration delay, const T& info, Exec&& e = Exec()):
			tpoint(Clock::now() + delay),
			id{info.Id},
			handler(nullptr),
			exec(std::move(e))
		{
			static_assert(sizeof(T) <= sizeof(Message::Data), "invalid payload size (T > Message_Default)");
			std::memcpy(&data, &info, sizeof(T));
		}

		void operator = (const Message& m) = delete;
		Message& operator = (Message&& m) = default;
		bool operator < (const Message& m) const noexcept;

		template <
			class T,
			ENABLE_IF((std::is_base_of<MsgBase<T>,T>{})),
			ENABLE_IF((std::enable_if_t<std::is_pointer<T>::value>{}))
		>
		operator T () {
			using TR = std::remove_pointer_t<T>;
			if(id == TR::Id)
				return payload<TR>();
			return nullptr;
		}
		template <
			class T,
			ENABLE_IF((std::is_base_of<MsgBase<T>,T>{})),
			ENABLE_IF((std::enable_if_t<std::is_pointer<T>::value>{}))
		>
		operator const T () const {
			T t = const_cast<Message*>(this)->operator T();
			return (const T)t;
		}
		template <class T>
		T* payload() {
			static_assert(sizeof(T) <= sizeof(Message::Data), "invalid payload size (T > Message_Default)");
			return reinterpret_cast<T*>(&data);
		}
		template <class T>
		const T* payload() const {
			return const_cast<Message*>(this)->payload<T>();
		}
	};

	class Looper;
	using Looper_SP = std::shared_ptr<Looper>;
	using MsgQueue = spi::pqueue<Message, std::deque>;
	class Looper : public std::enable_shared_from_this<Looper> {
		private:
			using Message_OP = spi::Optional<Message>;
			static TLS<Looper_SP>	tls_looper;
			bool			_bRun = true;
			MsgQueue		_msg;
			CondV			_cond;
			Mutex			_mutex;

			Message_OP _procMessage(std::function<bool (UniLock&)> lf);
		public:
			Looper() = default;
			Looper(const Looper&) = delete;
			Looper(Looper&& lp) noexcept;
			Looper& operator = (Looper&& lp) noexcept;
			// 呼び出したスレッドでキューがまだ用意されて無ければイベントキューを作成
			static void Prepare();
			// キューに溜まったメッセージをハンドラに渡す -> キューをクリア
			Message_OP wait();
			Message_OP peek(Milliseconds msec);
			static const Looper_SP& GetLooper();
			void pushEvent(Message&& m);
			void setState(bool bRun);
	};
}
