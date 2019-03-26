#pragma once
#include "../sound/depAL.hpp"
#include <cereal/access.hpp>

namespace rev {
	template <class Ar>
	void serialize(Ar& ar, SoundMgr_depAL& al) {
		ar(al._rate);
	}
	template <class Ar>
	void SoundMgr_depAL::load_and_construct(Ar& ar, cereal::construct<SoundMgr_depAL>& construct) {
		int rate;
		ar(rate);
		construct(rate);
	}
}
