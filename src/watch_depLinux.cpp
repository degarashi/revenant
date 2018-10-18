#include "watch_depLinux.hpp"
#include <unistd.h>
#include <sys/inotify.h>
#include "output.hpp"

namespace rev {
	// ---------------- FNotify_depLinux ----------------
	FNotify_depLinux::WatchThread::WatchThread():
		Thread<void (FNotify_depLinux*)>("WatchThread")
	{}
	namespace {
		constexpr int EVENTSIZE = sizeof(inotify_event);
	}
	void FNotify_depLinux::WatchThread::run(FNotify_depLinux* ths) {
		char buff[2048];
		for(;;) {
			::timeval tm;
			tm.tv_sec = 1;
			tm.tv_usec = 0;
			::fd_set fds;
			FD_ZERO(&fds);
			FD_SET(ths->_fd, &fds);
			FD_SET(ths->_cancelFd[0], &fds);
			const int ret = ::select(std::max(ths->_fd, ths->_cancelFd[0])+1, &fds, nullptr, nullptr, &tm);
			if(ret < 0) {
				LogRHere(Info,  "error (or signal caught?): %1%", ::strerror(errno));
				break;
			} else {
				if(FD_ISSET(ths->_fd, &fds)) {
					// inotify
					auto nread = read(ths->_fd, buff, sizeof(buff));
					if(nread < 0) {
						D_Assert("fd has set, but no data was given: %s", ::strerror(errno));
						break;
					}
					int ofs = 0;
					while(ofs < nread) {
						auto* e = reinterpret_cast<inotify_event*>(buff + ofs);
						ths->_pushInfo(*e);
						ofs += EVENTSIZE + e->len;
					}
				}
				if(FD_ISSET(ths->_cancelFd[0], &fds)) {
					// end
					LogRHere(Verbose,  "Ending mainloop.");
					break;
				}
			}
		}
	}

	FNotify_depLinux::FNotify_depLinux() {
		Assert((_fd = inotify_init()) >= 0, "failed to initialize inotify");
		::pipe(_cancelFd);
		_thread.start(this);
	}
	FNotify_depLinux::~FNotify_depLinux() {
		if(_thread.getStatus() != ThreadState::Idle) {
			const int endflag = ~0;
			write(_cancelFd[1], &endflag, sizeof(int));
			_thread.join();
		}
		close(_fd);
		close(_cancelFd[0]);
		close(_cancelFd[1]);
	}
	void FNotify_depLinux::_pushInfo(const inotify_event& e) {
		UniLock lk(_mutex);
		_eventL.push_back(Event{e.wd, DetectEvent(e.mask), e.name, e.cookie, static_cast<bool>(e.mask & IN_ISDIR)});
	}
	FNotify_depLinux::DSC FNotify_depLinux::addWatch(const std::string& path, const uint32_t mask) {
		const auto dsc = inotify_add_watch(_fd, path.c_str(), ConvertMask(mask));
		if(dsc < 0)
			throw AddWatchFailed(::strerror(errno));
		return dsc;
	}
	void FNotify_depLinux::remWatch(const DSC& dsc) {
		Expect(inotify_rm_watch(_fd, dsc) == 0, ::strerror(errno));
	}
	namespace {
		const static std::pair<FileEvent, int> c_flags[] = {
			{FileEvent::Create, IN_CREATE},
			{FileEvent::Modify, IN_MODIFY},
			{FileEvent::Remove, IN_DELETE},
			{FileEvent::MoveFrom, IN_MOVED_FROM},
			{FileEvent::MoveTo, IN_MOVED_TO},
			{FileEvent::Attribute, IN_ATTRIB},
			{FileEvent::Access, IN_ACCESS}
		};
	}
	uint32_t FNotify_depLinux::ConvertMask(const uint32_t mask) {
		uint32_t res = 0;
		for(auto& p : c_flags) {
			if(p.first & mask)
				res |= p.second;
		}
		return res | IN_DELETE_SELF | IN_MOVE_SELF;
	}
	FileEvent FNotify_depLinux::DetectEvent(const uint32_t mask) {
		for(auto& p : c_flags) {
			if(p.second & mask)
				return p.first;
		}
		return FileEvent::Invalid;
	}
	void FNotify_depLinux::procEvent(const CallbackD& cb) {
		UniLock lk(_mutex);
		for(auto& e : _eventL)
			cb(e);
		_eventL.clear();
	}
}
