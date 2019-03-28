#pragma once
#include "../uri/handler.hpp"
#include <cereal/types/vector.hpp>
#include <cereal/types/utility.hpp>

namespace rev {
	template <class Ar>
	void serialize(Ar& ar, UriHandlerV& u) {
		ar(u._handler);
	}
}
#include <cereal/types/polymorphic.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
CEREAL_REGISTER_TYPE(::rev::URIHandler);
