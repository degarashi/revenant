#pragma once
#include "../uri/uri.hpp"
#include "serialization/path.hpp"
#include <cereal/types/utility.hpp>
#include <cereal/types/vector.hpp>

namespace rev {
	template <class Ar>
	void serialize(Ar& ar, UserURI& uri) {
		ar(uri._name);
	}
	template <class Ar>
	void serialize(Ar& ar, FileURI& uri) {
		ar(uri._path);
	}
	template <class Ar>
	void serialize(Ar& ar, DataURI& uri) {
		ar(uri._mediaType, uri._bBase64, uri._data);
	}
}
namespace cereal {
	template <>
	struct LoadAndConstruct<::rev::FileURI> {
		template <class Ar>
		static void load_and_construct(Ar& ar, cereal::construct<::rev::FileURI>& construct) {
			::rev::PathBlock path;
			ar(path);
			construct(path.plain_utf8());
		}
	};
	template <>
	struct LoadAndConstruct<::rev::UserURI> {
		template <class Ar>
		static void load_and_construct(Ar& ar, cereal::construct<::rev::UserURI>& construct) {
			std::string name;
			ar(name);
			construct(name);
		}
	};
}
#include <cereal/types/polymorphic.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
CEREAL_REGISTER_TYPE(::rev::UserURI);
CEREAL_REGISTER_TYPE(::rev::FileURI);
CEREAL_REGISTER_TYPE(::rev::DataURI);
CEREAL_REGISTER_POLYMORPHIC_RELATION(::rev::URI, ::rev::UserURI);
CEREAL_REGISTER_POLYMORPHIC_RELATION(::rev::URI, ::rev::FileURI);
CEREAL_REGISTER_POLYMORPHIC_RELATION(::rev::URI, ::rev::DataURI);
