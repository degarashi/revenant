#pragma once
#include "serialization/urihandler.hpp"
#include "../uri/file.hpp"

namespace rev {
	template <class Ar>
	void serialize(Ar& ar, URI_File& uf) {
		ar(uf._basePath);
	}
}
namespace cereal {
	template <>
	struct LoadAndConstruct<::rev::URI_File> {
		template <class Ar>
		static void load_and_construct(Ar& ar, cereal::construct<::rev::URI_File>& construct) {
			::rev::PathBlock pb;
			ar(pb);
			construct(pb.plain_utf8());
		}
	};
}
CEREAL_REGISTER_TYPE(::rev::URI_File);
CEREAL_REGISTER_POLYMORPHIC_RELATION(::rev::URIHandler, ::rev::URI_File);
