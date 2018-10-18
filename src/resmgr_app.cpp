#include "resmgr_app.hpp"

namespace spi {
	::rev::URIWrap MakeAnonymous(::rev::URIWrap*, const uint64_t num) {
		return ::rev::URIWrap(::rev::IdURI(num));
	}
	bool IsAnonymous(const ::rev::URIWrap& key) {
		return key.uri->getType() == ::rev::URI::Type::Id;
	}
}
