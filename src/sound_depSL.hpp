#pragma once
#include "sound_common.hpp"
#include "spine/src/singleton.hpp"
#include "frea/src/vector.hpp"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

namespace cereal {
	template <class T>
	class construct;
}
namespace rev {
	std::string GetIIDString(const SLInterfaceID& iid);
	SLmillibel VolToMillibel(float vol);
	float MillibelToVol(SLmillibel mv);
	using Vec3 = frea::Vec3;

	class SLObj {
		private:
			SLObjectItf	_obj;
		public:
			SLObj(SLObj&& s);
			SLObj(const SLObj&) = delete;
			SLObj(SLObjectItf itf=nullptr);
			virtual ~SLObj();
			SLObjectItf& refObj();
			void realize(bool async);
			void resume(bool async);
			SLuint32 getState();
			template <class ITF>
			ITF getInterface(const SLInterfaceID& id) {
				ITF itf;
				SLEC_M(Trap, _obj, GetInterface, id, &itf);
				return itf;
			}
			template <class ITF>
			ITF tryGetInterface(const SLInterfaceID& id) {
				ITF itf;
				auto res = (*_obj)->GetInterface(_obj, id, &itf);
				if(res == SL_RESULT_SUCCESS)
					return itf;
				return nullptr;
			}
	};

	// OpenSLはユーザーが管理
	class ABuffer_depSL {
		private:
			constexpr static int HasBuffer = 1;
			ByteBuff	_buff;
		public:
			void writeBuffer(const AFormatF& af, const void* src, std::size_t len);
			const ByteBuff& getBuff() const;
	};
	using ABufferDep = ABuffer_depSL;

	class ASource_depSL {
		private:
			SLObj				_aplayer;
			SLPlayItf			_playItf;
			SLVolumeItf			_volItf;
			SLBufferQueueItf	_bqItf;
			SLPlaybackRateItf	_rateItf;
			SLpermille			_rateMin, _rateMax, _rateStep;
			SLuint32			_blockCount;
		public:
			ASource_depSL();
			ASource_depSL(ASource_depSL&& s);
			ASource_depSL(const ASource_depSL&) = delete;

			void play();
			void reset();
			void pause();
			void update(bool bPlaying);
			void setGain(float vol);
			void setPitch(float pitch);
			float timeTell(float def);
			int64_t pcmTell(int64_t def);
			void timeSeek(float t);
			void pcmSeek(int64_t p);

			void enqueue(ABuffer_depSL& buff);
			int getUsedBlock();
			void clearBlock();
			bool isEnded();

			// ---- 未対応 ----
			void setRelativeMode(bool bRel) {}
			void setPosition(const Vec3& pos) {}
			void setDirection(const Vec3& dir) {}
			void setVelocity(const Vec3& vel) {}
			void setGainRange(float gmin, float gmax) {}
			void setAngleGain(float inner, float outer) {}
			void setAngleOuterGain(float gain) {}
	};
	using ASourceDep = ASource_depSL;

	//! OpenSLのデバイス初期化等
	class SoundMgr_depSL : public spi::Singleton<SoundMgr_depSL> {
		private:
			// Androidでは何もしなくてもスレッドセーフなのでコンテキスト管理は必要ない
			SLObj			_engine;
			SLEngineItf 	_engineItf;
			SLObj			_outmix;
			SDLAFormatCF	_outFormat;

			template <class Ar>
			friend void serialize(Ar&, SoundMgr_depSL&);
			template <class Ar>
			static void load_and_construct(Ar&, cereal::construct<SoundMgr_depSL>&);

		public:
			SoundMgr_depSL(int rate);
			SoundMgr_depSL(const SoundMgr_depSL&) = delete;
			int getRate() const;

			void printVersions(std::ostream& os);
			SLEngineItf	getEngineItf() const;
			SLObj& getOutMix();
			const SDLAFormatCF& getOutMixFormat() const;

			// OpenSLはスレッドセーフなのでこれらのメソッドはダミー
			void makeCurrent() {}
			void resetCurrent() {}
			void suspend() {}
			void process() {}
	};
	using SoundMgrDep = SoundMgr_depSL;
}
