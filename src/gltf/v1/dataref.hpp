#pragma once
#include "gltf/v1/dataquery_if.hpp"
#include "../dataref.hpp"

namespace rev::gltf::v1 {
	#define DEF_REF(z, ign, name) \
		using BOOST_PP_CAT(DRef_, name) = gltf::DataRef<name, Tag, IDataQuery, &IDataQuery::BOOST_PP_CAT(get, name)>;
	BOOST_PP_SEQ_FOR_EACH(DEF_REF, EMPTY, SEQ_V1_RES)
	#undef DEF_REF
}
