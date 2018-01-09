#pragma once
#include "serialization/urihandler.hpp"
#include "../uri_data.hpp"

namespace rev {
	template <class Ar>
	void serialize(Ar& ar, URI_Data&) {}
}
CEREAL_REGISTER_TYPE(::rev::URI_Data);
CEREAL_REGISTER_POLYMORPHIC_RELATION(::rev::URIHandler, ::rev::URI_Data);
