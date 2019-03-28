#pragma once
#include <string>
#include <memory>
#include <map>
#include "spine/src/enum.hpp"
#include <ostream>

namespace rev {
	using String_SP = std::shared_ptr<std::string>;
	class Dir;
	using Dir_SP = std::shared_ptr<Dir>;
	struct UserData {
		virtual ~UserData() {}
	};
	using Data_SP = std::shared_ptr<UserData>;

	// ファイル構造検知
	DefineEnumPair(
		FileEvent,
		((Invalid)(0x00))
		((Create)(0x01))
		((Access)(0x02))
		((Modify)(0x04))
		((Remove)(0x08))
		((MoveFrom)(0x10))
		((MoveTo)(0x20))
		((Attribute)(0x40))
	);
	struct FEv;
	using Callback = std::function<void (FEv&)>;

	template <class T>
	struct FNotifyEvent {
		using value_t = T;
		value_t			dsc;
		FileEvent		event;
		std::string		path;
		uint32_t		cookie;
		bool			bDir;
	
		FNotifyEvent() = default;
		FNotifyEvent(const FNotifyEvent&) = default;
		FNotifyEvent(FNotifyEvent&&) = default;
		template <class T2, class T3>
		FNotifyEvent(T2&& ds, const FileEvent& ev, T3&& p, const uint32_t c, const bool dir):
			dsc(std::forward<T2>(ds)),
			event(ev),
			path(std::forward<T3>(p)),
			cookie(c),
			bDir(dir)
		{}
		std::ostream& print(std::ostream& os) const {
			using std::endl;
			os << "FNotifyEvent: [" << endl
				<< "value: " << dsc << endl
				<< "event: " << event << endl
				<< "path: " << path << endl
				<< "cookie: " << cookie << endl
				<< "bDir: " << bDir << endl
				<< "]";
			return os;
		}
	};
	template <class T>
	std::ostream& operator << (std::ostream& os, const FNotifyEvent<T>& ntf) {
		return ntf.print(os);
	}
	struct AddWatchFailed : std::runtime_error {
		using std::runtime_error::runtime_error;
	};
}
