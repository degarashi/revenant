#include "rev_output.hpp"
#include "sdl_tls.hpp"
#include <boost/format.hpp>

namespace rev {
	namespace log {
		void RevOutput::print(const lubee::log::Type::e type, const std::string& s) {
			std::string str;
			// スレッド名を出力
			boost::format msg("[%1%]:\n%2%");
			if(tls_threadName.initialized())
				str = (msg % *tls_threadName % s).str();
			else {
				const auto thId = SDL_GetThreadID(nullptr);
				str = (msg % thId % s).str();
			}
			// スレッド間で出力が混ざらないようにロックをかける
			const UniLock lk(_mutex);
			base::print(type, str);
		}
	}
}
