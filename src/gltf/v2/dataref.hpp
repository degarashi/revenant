#pragma once
#include "gltf/v2/dataquery_if.hpp"
#include "../dataref.hpp"

namespace rev::gltf::v2 {
	#define DEF_REF(z, ign, name) \
		using BOOST_PP_CAT(DRef_, name) = gltf::DataRef<name, DataId, IDataQuery, &IDataQuery::BOOST_PP_CAT(get, name)>; \
		using BOOST_PP_CAT(BOOST_PP_CAT(DRef_, name), _OP) = spi::Optional<BOOST_PP_CAT(DRef_, name)>;
	BOOST_PP_SEQ_FOR_EACH(DEF_REF, EMPTY, SEQ_V2_RES)
	#undef DEF_REF
}
