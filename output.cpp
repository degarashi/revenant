#include "output.hpp"
#include "sdl_tls.hpp"
#include <boost/format.hpp>

namespace rev {
	namespace log {
		void RevOutput::print(const lubee::log::Type::e type, const std::string& s) {
			// スレッド名を出力
			boost::format msg("[%1%]: %2%");
			if(tls_threadName.initialized())
				base::print(type, (msg % *tls_threadName % s).str());
			else {
				const auto thId = SDL_GetThreadID(nullptr);
				base::print(type, (msg % thId % s).str());
			}
		}
	}
}
