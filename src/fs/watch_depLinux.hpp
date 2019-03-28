#pragma once
#include "../sdl/thread.hpp"
#include "watch_common.hpp"
#include <vector>

struct inotify_event;
namespace rev {
	class FNotify_depLinux {
		public:
			using Event = FNotifyEvent<int>;
		private:
			Mutex					_mutex;
			int						_fd,
									_cancelFd[2];
			//! ファイル監視用スレッド
			class WatchThread : public Thread<void (FNotify_depLinux*)> {
				protected:
					void run(FNotify_depLinux* self) override;
				public:
					WatchThread();
			};
			WatchThread				_thread;
			using EventL = std::vector<Event>;
			EventL		_eventL;

			static void ASyncLoop(FNotify_depLinux* ths);
			void _pushInfo(const inotify_event& e);

		public:
			using DSC = int;
			using CallbackD = std::function<void (const Event&)>;

			FNotify_depLinux();
			FNotify_depLinux(FNotify_depLinux&&) = delete;
			~FNotify_depLinux();
			DSC addWatch(const std::string& path, uint32_t mask);
			void remWatch(const DSC& dsc);
			void procEvent(const CallbackD& cb);
			static uint32_t ConvertMask(uint32_t mask);
			static FileEvent DetectEvent(uint32_t mask);
	};
	using FNotifyDep = FNotify_depLinux;
}
