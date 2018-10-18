#pragma once
#include "gltf/v2/resource.hpp"
#include "gltf/dataquery_if_base.hpp"

namespace rev::gltf::v2 {
	using DataId = uint_fast32_t;
	using ByteV = std::vector<uint8_t>;
	struct IDataQuery : gltf::IDataQueryBase {
		using Tag_t = DataId;
		#define DEF_METHOD(z, ign, name) \
			virtual const name& BOOST_PP_CAT(get, name)(const DataId& id) const { \
				throw UnknownID("Unknown id: "s + std::to_string(id)); }
		BOOST_PP_SEQ_FOR_EACH(DEF_METHOD, EMPTY, SEQ_V2_RES)
		#undef DEF_METHOD
		virtual const ByteV& getBinary() const { throw 0; }
	};
	struct IDataQuery_Nest : IDataQuery {
		const IDataQuery& _base;
		IDataQuery_Nest(const IDataQuery& base):
			_base(base)
		{}
		#define DEF_METHOD(z, ign, name) \
			const name& BOOST_PP_CAT(get, name)(const DataId& id) const override { \
				return _base.BOOST_PP_CAT(get, name)(id); }
		BOOST_PP_SEQ_FOR_EACH(DEF_METHOD, EMPTY, SEQ_V2_RES)
		#undef DEF_METHOD
		PathBlock getFilePath(const Path& p) const override {
			return _base.getFilePath(p);
		}
	};
}
