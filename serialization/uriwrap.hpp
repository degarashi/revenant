#pragma once
#include "uri.hpp"
#include "../resmgr_app.hpp"
#include <cereal/cereal.hpp>

namespace rev {
	template <class Ar>
	void serialize(Ar& ar, URIWrap& w) {
		ar(w.uri);
	}
}
