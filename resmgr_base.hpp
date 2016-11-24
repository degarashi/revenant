#pragma once
#include "urihandler.hpp"

namespace rev {
	using Priority = uint32_t;
	class URI;
	class ResMgrBase {
		private:
			static std::vector<ResMgrBase*>		s_mgr;
			UriHandlerV							_handler;
		public:
			virtual ~ResMgrBase();
			virtual HRes loadResource(const URI& uri) = 0;
			static HRes LoadResource(const URI& uri);
	};
}
