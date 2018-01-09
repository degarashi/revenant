#pragma once
#include "handle.hpp"
#include <memory>
#include <algorithm>

namespace rev {
	struct Access {
		enum e : int {
			Read = 0x01,
			Write = 0x02,
			Binary = 0x04
		};
	};
	class URI;
	struct URIHandler {
		virtual HRW openURI(const URI& uri, int access) const = 0;
		virtual ~URIHandler() {}
	};
	using UriHandler_SP = std::shared_ptr<URIHandler>;

	class UriHandlerV {
		private:
			using Priority = uint32_t;
			using HandlerPair = std::pair<Priority, UriHandler_SP>;
			using HandlerVec = std::vector<HandlerPair>;
			template <class Ar>
			friend void serialize(Ar&, UriHandlerV&);

			HandlerVec	_handler;
			auto _findHandler(const UriHandler_SP& h) const;
		public:
			void addHandler(const Priority prio, const UriHandler_SP& h);
			void remHandler(const UriHandler_SP& h);
			HRW procHandler(const URI& uri, const int access) const;
			HRW procHandler(const std::string& uri, const int access) const ;
			void clearHandler();
	};
}
