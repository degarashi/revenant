#pragma once
#include "gltf/v1/resource.hpp"
#include "gltf/dataquery_if_base.hpp"

namespace rev::gltf::v1 {
	using Tag = std::string;
	struct IDataQuery : gltf::IDataQueryBase {
		#define DEF_METHOD(z, ign, name) \
			virtual const name& BOOST_PP_CAT(get, name)(const Tag& tag) const { \
				throw UnknownID("Unknown id: "s + tag); }
		BOOST_PP_SEQ_FOR_EACH(DEF_METHOD, EMPTY, SEQ_RES)
		#undef DEF_METHOD
	};
	struct IDataQuery_Nest : IDataQuery {
		const IDataQuery& _base;
		IDataQuery_Nest(const IDataQuery& base):
			_base(base)
		{}
		#define DEF_METHOD(z, ign, name) \
			const name& BOOST_PP_CAT(get, name)(const Tag& tag) const override { \
				return _base.BOOST_PP_CAT(get, name)(tag); }
		BOOST_PP_SEQ_FOR_EACH(DEF_METHOD, EMPTY, SEQ_RES)
		#undef DEF_METHOD
		PathBlock getFilePath(const Path& p) const override {
			return _base.getFilePath(p);
		}
	};
}
