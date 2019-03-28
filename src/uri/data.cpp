#include "data.hpp"
#include "uri.hpp"
#include "../sdl/rw.hpp"

namespace rev {
	HRW URI_Data::openURI(const URI& uri, const int access) const {
		if(uri.getType() == URI::Type::Data) {
			if(!(access & Access::Write)) {
				auto& data = static_cast<const DataURI&>(uri).data();
				return mgr_rw.fromIterator(data.cbegin(), data.cend());
			}
		}
		return HRW();
	}
}
