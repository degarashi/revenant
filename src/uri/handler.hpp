#pragma once
#include "../handle/sdl.hpp"
#include "../handle/uri.hpp"
#include <algorithm>
#include <vector>

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

	class UriHandlerV {
		private:
			using Priority = uint32_t;
			using HandlerPair = std::pair<Priority, HURIHandler>;
			using HandlerVec = std::vector<HandlerPair>;
			template <class Ar>
			friend void serialize(Ar&, UriHandlerV&);

			HandlerVec	_handler;
			auto _findHandler(const HURIHandler& h) const;
		public:
			void addHandler(const Priority prio, const HURIHandler& h);
			void remHandler(const HURIHandler& h);
			HRW procHandler(const URI& uri, const int access) const;
			HRW procHandler(const std::string& uri, const int access) const ;
			void clearHandler();
	};
}
