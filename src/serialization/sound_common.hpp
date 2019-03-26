#pragma once
#include "../sound/common.hpp"
#include <cereal/types/base_class.hpp>

namespace rev {
	template <class Ar>
	void serialize(Ar& ar, SDLAFormatCF& cf) {
		ar(cereal::base_class<SDLAFormat>(&cf), cf.channels, cf.freq);
	}
	template <class Ar>
	void serialize(Ar& ar, AFormatF& af) {
		ar(cereal::base_class<AFormat>(&af), af.freq);
	}
	template <class Ar>
	void load(Ar& ar, VorbisFile& v) {
		ar(v._hRW, v._initialFPos);
		v._init();
		int64_t fpos;
		ar(fpos);
		v.pcmSeek(fpos);
	}
	template <class Ar>
	void save(Ar& ar, const VorbisFile& v) {
		ar(v._hRW, v._initialFPos);
		int64_t fpos = v.pcmTell();
		ar(fpos);
	}
}
namespace cereal {
	template <class Ar>
	struct specialize<Ar, ::rev::SDLAFormatCF, cereal::specialization::non_member_serialize> {};
	template <class Ar>
	struct specialize<Ar, ::rev::AFormatF, cereal::specialization::non_member_serialize> {};
}
