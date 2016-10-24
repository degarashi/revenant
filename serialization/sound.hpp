#pragma once
#include "sound.hpp"
#include "serialization/sound_depAL.hpp"
#include "serialization/sound_common.hpp"
#include "serialization/resmgr_app.hpp"
#include "serialization/sdl_rw.hpp"
#include "spine/serialization/resmgr.hpp"
#include <cereal/types/base_class.hpp>
#include <cereal/access.hpp>
#include <cereal/types/chrono.hpp>
#include <cereal/types/vector.hpp>

namespace rev {
	template <class Ar>
	void serialize(Ar& ar, ABuffer& ab) {
		ar(ab._format, ab._duration);
	}
	template <class Ar>
	void serialize(Ar& ar, AWaveBatch& a) {
		ar(cereal::base_class<ABuffer>(&a), a._buff);
	}
	template <class Ar>
	void serialize(Ar& ar, AOggBatch& a) {
		ar(cereal::base_class<ABuffer>(&a), a._buff);
	}
	template <class Ar>
	void serialize(Ar& ar, AOggStream& a) {
		ar(cereal::base_class<ABuffer>(&a), a._vfile, a._prevOffset);
	}
	template <class Ar>
	void serialize(Ar& ar, ASource::Fade& f) {
		ar(f.durBegin, f.durEnd, f.fromGain, f.toGain, f.callback);
	}
	template <class Ar>
	void serialize(Ar& ar, ASource::Save& s) {
		ar(s.hAb, s.stateID, s.nLoop, s.currentGain, s.targetGain,
			s.fadeInTime, s.fadeOutTime, s.timePos);
	}
	template <class Ar>
	void save(Ar& ar, const ASource& s) {
		ar(s._hAb);
		for(auto& p : s._fade)
			serialize(ar, const_cast<ASource::Fade&>(p));
		ASource::Save save;
		// 保存前に再生位置を更新
		auto tm = s._timePos;
		const_cast<Duration&>(s._timePos) = s._timePos + (Clock::now() - s._tmUpdate);
		save.pack(s);
		ar(save);
		std::cout << "save: " << s._playedCur << std::endl;
		const_cast<Duration&>(s._timePos) = tm;
	}
	template <class Ar>
	void load(Ar& ar, ASource& s) {
		ar(s._hAb);
		for(auto& p : s._fade)
			serialize(ar, p);
		ASource::Save save;
		ar(save);
		save.unpack(s);
		std::cout << "load: " << s._playedCur << std::endl;
	}
	template <class Ar>
	void serialize(Ar& ar, AGroup& a) {
		ar(a._source, a._nActive, a._bPaused);
	}
	template <class Ar>
	void serialize(Ar& ar, SoundMgr& mgr) {
		mgr.makeCurrent();
		try {
			ar(mgr.getRate(), mgr._buffMgr, mgr._srcMgr, mgr._sgMgr);
		} catch(const std::exception& e) {
			std::cout << e.what() << std::endl;
		}
	}
	template <class Ar>
	void serialize(Ar& ar, ABufMgr& mgr) {
		ar(cereal::base_class<ResMgrApp<ABuffer>>(&mgr));
	}
}
namespace cereal {
	template <>
	struct LoadAndConstruct<::rev::SoundMgr> {
		template <class Ar>
		static void load_and_construct(Ar& ar, cereal::construct<::rev::SoundMgr>& construct) {
			int rate;
			ar(rate);
			construct(rate);
			try {
				ar(construct->_buffMgr, construct->_srcMgr, construct->_sgMgr);
			} catch(const std::exception& e) {
				std::cout << e.what() << std::endl;
			}
		}
	};
	template <class Ar>
	struct specialize<Ar, ::rev::ABufMgr, cereal::specialization::non_member_serialize> {};
}
#include <cereal/types/polymorphic.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
CEREAL_REGISTER_TYPE(::rev::ABuffer);
CEREAL_REGISTER_TYPE(::rev::AWaveBatch);
CEREAL_REGISTER_TYPE(::rev::AOggBatch);
CEREAL_REGISTER_TYPE(::rev::AOggStream);
CEREAL_REGISTER_POLYMORPHIC_RELATION(::rev::ABuffer, ::rev::AWaveBatch);
CEREAL_REGISTER_POLYMORPHIC_RELATION(::rev::ABuffer, ::rev::AOggBatch);
CEREAL_REGISTER_POLYMORPHIC_RELATION(::rev::ABuffer, ::rev::AOggStream);
