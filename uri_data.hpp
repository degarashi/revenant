#pragma once
#include "urihandler.hpp"
#include "abstbuffer.hpp"

namespace rev {
	//! Base64によるデータURI
	class URI_Data : public URIHandler {
		private:
			template <class Ar>
			friend void serialize(Ar&, URI_Data&);
		public:
			HRW openURI(const URI& uri, int access) const override;
	};
}
