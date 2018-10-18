#pragma once
#include "sdl_mutex.hpp"

namespace rev {
	namespace log {
		//! 複数スレッドからの同時出力にMutexにて対応
		class RevOutput : public lubee::log::DefaultOutput {
			private:
				Mutex	_mutex;
				using base = lubee::log::DefaultOutput;
			public:
				void print(const lubee::log::Type::e type, const std::string& s) override;
		};
	}
}
