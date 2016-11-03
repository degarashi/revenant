#include "../glx.hpp"

namespace rev {
	template <class Ar>
	void serialize(Ar& ar, VDecl::VDInfo& vd) {
		ar(
			CEREAL_NVP(streamID),
			CEREAL_NVP(offset),
			CEREAL_NVP(elemFlag),
			CEREAL_NVP(bNormalize),
			CEREAL_NVP(elemSize),
			CEREAL_NVP(semID)
		);
	}

	template <class Ar>
	void load(Ar& ar, VDecl& vd) {
		ar(cereal::make_nvp("vdInfo", vd._vdInfo));
		_init();
	}
	template <class Ar>
	void save(Ar& ar, const VDecl& vd) {
		ar(cereal::make_nvp("vdInfo", vd._vdInfo));
	}
}
