#pragma once
#include "common.hpp"
#include "clock.hpp"
#include "frea/src/vector.hpp"
#include "spine/src/singleton.hpp"
#include "../sdl/tls.hpp"
#include "../sdl/mutex.hpp"
#include <AL/al.h>
#include <AL/alc.h>

namespace cereal {
	template <class T>
	class construct;
}
namespace rev {
	using Vec3 = frea::Vec3;
	ALenum AsALFormat(const AFormat& f) noexcept;

	// OpenALはライブラリに確保したバッファに転送してから使う
	class ABuffer_depAL {
		private:
			constexpr static int HasBuffer = 0;
			// OpenALではライブラリがバッファ実体を管理するのでここにはIDだけ保存
			ALuint	_id;
		public:
			ABuffer_depAL();
			ABuffer_depAL(ABuffer_depAL&& a);
			ABuffer_depAL(const ABuffer_depAL&) = delete;
			~ABuffer_depAL();
			void writeBuffer(const AFormatF& af, const void* src, size_t len);
			ALuint getID() const;
	};
	using ABufferDep = ABuffer_depAL;

	// Bufferはあくまでデータ (batch or streaming)
	// Sourceがストリーミング制御
	class ASource_depAL {
		private:
			enum class AState {
				Initial,
				Playing,
				Paused,
				Stopped,
			};
			ALuint	_id;

		public:
			ASource_depAL();
			ASource_depAL(ASource_depAL&& a) noexcept;
			ASource_depAL(const ASource_depAL&) = delete;
			~ASource_depAL();

			void play();
			void reset();
			void pause();
			void update(bool bPlaying);
			void setGain(float vol);
			void setPitch(float pitch);
			Duration timeTell(Duration def);
			int64_t pcmTell(int64_t def);
			ALuint getID() const;
			void timeSeek(float t);
			void pcmSeek(int64_t p);

			void enqueue(const ABuffer_depAL& buff);
			int getUsedBlock();
			void clearBlock();

			// ---- ASourceでは未対応だけど一応実装 ----
			void setRelativeMode(bool bRel);
			void setPosition(const Vec3& pos);
			void setDirection(const Vec3& dir);
			void setVelocity(const Vec3& vel);
			void setGainRange(float gmin, float gmax);
			void setAngleGain(float inner, float outer);
			void setAngleOuterGain(float gain);
	};
	using ASourceDep = ASource_depAL;

	//! OpenALのデバイス初期化等
	class SoundMgr_depAL : public spi::Singleton<SoundMgr_depAL> {
		private:
			static SDL_atomic_t s_atmThCounter;
			static TLS<int> s_thID;

			// スレッドごとにコンテキストを用意
			// (OpenALはスレッドセーフらしいので後で変える)
			using CtxL = std::vector<ALCcontext*>;
			CtxL 		_context;
			Mutex		_mutex;
			ALCdevice*	_device;
			int			_rate;

			template <class Ar>
			friend void serialize(Ar&, SoundMgr_depAL&);
			template <class Ar>
			static void load_and_construct(Ar&, cereal::construct<SoundMgr_depAL>&);

		public:
			SoundMgr_depAL(int rate);
			SoundMgr_depAL(const SoundMgr_depAL&) = delete;
			~SoundMgr_depAL();
			// デバッグ用 (シリアライズされる変数のみ比較)
			bool operator == (const SoundMgr_depAL& m) const noexcept;
			bool operator != (const SoundMgr_depAL& m) const noexcept;

			void printVersions(std::ostream& os);
			ALCdevice* getDevice() const;
			int getRate() const;

			// スレッドに関連付けられたコンテキストが無ければ生成
			void makeCurrent();
			void resetCurrent();
			void suspend();
			void process();

			// ---- Listener functions ----
			static void SetPosition(const Vec3& pos);
			static void SetVelocity(const Vec3& v);
			static void SetGain(float g);
			static void SetOrientation(const Vec3& dir, const Vec3& up);
			static void SetDopplerFactor(float f);
			static void SetSpeedOfSound(float s);

			enum class DistModel : ALenum {
				InvDist = AL_INVERSE_DISTANCE,
				InvDistClamp = AL_INVERSE_DISTANCE_CLAMPED,
				LinearDist = AL_LINEAR_DISTANCE,
				LinearDistClamp = AL_LINEAR_DISTANCE_CLAMPED,
				ExponentDist = AL_EXPONENT_DISTANCE,
				ExponentDistClamp = AL_EXPONENT_DISTANCE_CLAMPED,
				Nothing = AL_NONE
			};
			static void SetDistModel(DistModel model);
	};
	using SoundMgrDep = SoundMgr_depAL;
}
