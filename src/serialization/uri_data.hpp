#pragma once
#include "serialization/urihandler.hpp"
#include "../uri/data.hpp"

namespace rev {
	template <class Ar>
	void serialize(Ar&, URI_Data&) {}
}
CEREAL_REGISTER_TYPE(::rev::URI_Data);
CEREAL_REGISTER_POLYMORPHIC_RELATION(::rev::URIHandler, ::rev::URI_Data);
