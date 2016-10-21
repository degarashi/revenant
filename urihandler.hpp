#pragma once
#include "handle.hpp"
#include <memory>
#include <algorithm>
#include <iostream>

namespace rev {
	using OStream_UP = std::unique_ptr<std::ostream>;
	using IStream_UP = std::unique_ptr<std::istream>;

	struct Access {
		enum e : int {
			Read = 0x01,
			Write = 0x02,
			Binary = 0x04
		};
	};
	class URI;
	struct URIHandler {
		virtual HRW openURI(const URI& uri, int access) = 0;
		virtual ~URIHandler() {}
	};
	using UriHandler_SP = std::shared_ptr<URIHandler>;

	class UriHandlerV {
		private:
			using Priority = uint32_t;
			using HandlerPair = std::pair<Priority, UriHandler_SP>;
			using HandlerVec = std::vector<HandlerPair>;

			HandlerVec	_handler;
			auto _findHandler(const UriHandler_SP& h) const;
		public:
			void addHandler(const Priority prio, const UriHandler_SP& h);
			void remHandler(const UriHandler_SP& h);
			HRW procHandler(const URI& uri, const int access) const;
			void clearHandler();
	};
}
