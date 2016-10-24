#pragma once
#include "../sound_depSL.hpp"
#include <cereal/access.hpp>

namespace rev {
	template <class Ar>
	void serialize(Ar& ar, SoundMgr_depSL& sl) {
		ar(sl._outFormat.freq);
	}
	template <class Ar>
	void SoundMgr_depSL::load_and_construct(Ar& ar, cereal::construct<SoundMgr_depSL>& construct) {
		int rate;
		ar(rate);
		construct(rate);
	}
}
