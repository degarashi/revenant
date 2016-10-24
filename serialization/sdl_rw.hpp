#pragma once
#include "../sdl_rw.hpp"
#include "serialization/uri.hpp"
#include "spine/serialization/resmgr.hpp"
#include "serialization/urihandler.hpp"
#include <cereal/types/vector.hpp>

namespace rev {
	template <class Ar>
	void serialize(Ar& /*ar*/, RWops::TempData& /*tmp*/) {
		D_Expect(false, "this object cannot serialize");
	}

	template <class Ar>
	void load(Ar& ar, RWops::VectorData& d) {
		int64_t pos;
		ar(d._uri, d._buff, pos);
		d._deserializeFromData(pos);
	}
	template <class Ar>
	void save(Ar& ar, const RWops::VectorData& d) {
		const int64_t pos = d.tell();
		ar(d._uri, d._buff, pos);
	}

	template <class Ar>
	void load(Ar& ar, RWops::FileData& d) {
		int64_t pos;
		ar(d._path, d._access, pos);
		d._deserializeFromData(pos);
	}
	template <class Ar>
	void save(Ar& ar, const RWops::FileData& d) {
		const int64_t pos = d.tell();
		ar(d._path, d._access, pos);
	}

	template <class Ar>
	void serialize(Ar& ar, RWops& ops) {
		ar(ops._access, ops._data, ops._endCB);
	}
	template <class Ar>
	void serialize(Ar& ar, RWMgr& mgr) {
		ar(
			mgr._orgName,
			mgr._appName,
			cereal::base_class<typename RWMgr::base_t>(&mgr),
			cereal::base_class<UriHandlerV>(&mgr)
		);
	}
}
namespace cereal {
	template <>
	struct LoadAndConstruct<::rev::RWMgr> {
		template <class Ar>
		static void load_and_construct(Ar& ar, cereal::construct<::rev::RWMgr>& construct) {
			std::string org, app;
			ar(org, app);
			construct(org, app);
			ar(
				cereal::base_class<typename ::rev::RWMgr::base_t>(construct.ptr()),
				cereal::base_class<::rev::UriHandlerV>(construct.ptr())
			);
		}
	};
	template <class Ar>
	struct specialize<Ar, ::rev::RWMgr, cereal::specialization::non_member_serialize> {};
}
#include <cereal/types/polymorphic.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
CEREAL_REGISTER_TYPE(::rev::RWops::TempData);
CEREAL_REGISTER_TYPE(::rev::RWops::VectorData);
CEREAL_REGISTER_TYPE(::rev::RWops::FileData);
CEREAL_REGISTER_POLYMORPHIC_RELATION(::rev::RWops::Data, ::rev::RWops::TempData);
CEREAL_REGISTER_POLYMORPHIC_RELATION(::rev::RWops::Data, ::rev::RWops::VectorData);
CEREAL_REGISTER_POLYMORPHIC_RELATION(::rev::RWops::Data, ::rev::RWops::FileData);
