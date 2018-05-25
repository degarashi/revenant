#pragma once
#include "gltf/resource.hpp"
#include "gltf/exceptions.hpp"
#include "../path.hpp"

namespace rev::gltf {
	using std::literals::operator""s;
	using Tag = std::string;
	using Path = std::string;
	struct IDataQuery {
		#define DEF_METHOD(z, ign, name) \
			virtual const name& BOOST_PP_CAT(get, name)(const Tag& tag) const { \
				throw UnknownID("Unknown id: "s + tag); }
		BOOST_PP_SEQ_FOR_EACH(DEF_METHOD, EMPTY, SEQ_RES)
		#undef DEF_METHOD
		virtual PathBlock getFilePath(const Path& p) const {
			throw UnknownID("Unknown id: "s + p);
		}
		virtual ~IDataQuery() {}
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
