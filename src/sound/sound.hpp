#pragma once
#include SOUND_HEADER
#include "clock.hpp"
#include "handle/sound.hpp"
#include "apppath.hpp"
#include "resmgr_app.hpp"
#include "spine/src/resmgr.hpp"
#include "../sdl/rw.hpp"

namespace cereal {
	class access;
	template <class T>
	struct LoadAndConstruct;
}
namespace rev {
	Duration CalcTimeLength(int word_size, int ch, int hz, std::size_t buffLen) noexcept;
	uint64_t CalcSampleLength(int word_size, int ch, int hz, Duration dur) noexcept;
	uint64_t CalcSampleLength(const AFormatF& af, Duration dur) noexcept;

	//! 共有サンプルデータソース
	class ABuffer : public Resource {
		protected:
			AFormatF	_format;
			Duration	_duration;
			ABuffer();
		private:
			template <class Ar>
			friend void serialize(Ar&, ABuffer&);
		public:
			virtual ~ABuffer() {}
			virtual bool isStreaming() const = 0;
			// 指定の場所から任意サイズのサンプルデータを受け取る
			/*! \param[in] offset	取得したいサンプルのオフセット(bytes)
				\param[in] buffLen	受信バッファサイズ(bytes)
				\return コピーされたデータサイズ(bytes)  */
			virtual std::size_t getData(void* /*dst*/, uint64_t /*offset*/, std::size_t /*buffLen*/) const {
				Assert0(false);
				throw 0;
			}
			/*! \param[in] offset 受信オフセット(bytes) */
			virtual std::pair<const void*, std::size_t> getAllData(uint64_t /*offset*/) const {
				Assert0(false);
				throw 0;
			}
			Duration getDuration() const noexcept { return _duration; }
			const AFormatF& getFormat() const noexcept { return _format; }
			virtual void invalidate() noexcept {}
			const char* getResourceName() const noexcept override;
	};
	class ABufMgr : public ResMgrApp<ABuffer>, public spi::Singleton<ABufMgr> {
		private:
			enum ResourceType {
				Sound,
				_Num
			};
			const static std::string cs_rtname[ResourceType::_Num];
			template <class Ar>
			friend void serialize(Ar&, ABufMgr&);
		public:
			ABufMgr();
	};

	//! 固有サンプルデータソース
	class ABufSub {
		private:
			HAb			_hAb;
			using UPBuff = std::unique_ptr<ABufferDep[]>;
			UPBuff		_abuff;
			uint32_t	_nLoop, _nLoopInit;
			int			_nBuffer;	//!< AudioBufferの数
			int			_readCur,	//!< GetBlockで送り出した位置
						_playedCur,	//!< 再生が終わった位置
						_writeCur;	//!< 未再生バッファの準備ができている位置
			uint64_t 	_offset;

			void _fillBuffer();
		public:
			ABufSub(const HAb& hAb, uint32_t nLoop);
			ABufSub(const ABufSub&) = delete;
			ABufSub(ABufSub&& a) = default;
			ABufSub& operator = (ABufSub&&) = default;

			ABufferDep* getBlock();
			void rewind();
			bool isEOF();
			//! 再生が終わったブロック番号のセット (from ASource)
			void setPlayedCursor(int cur);
			void timeSeek(Duration tp);
			void pcmSeek(uint64_t t);

			Duration getDuration() const;
			const AFormatF& getFormat() const;
			void invalidate();
	};

	// Waveバッチ再生
	// リストアは再度ファイルから読み込む
	class AWaveBatch : public ABuffer {
		private:
			ByteBuff	_buff;

			friend class cereal::access;
			template <class Ar>
			friend void serialize(Ar&, AWaveBatch&);
			AWaveBatch() = default;
		public:
			AWaveBatch(const HRW& hRW);
			bool isStreaming() const override;
			std::pair<const void*, std::size_t> getAllData(uint64_t offset) const override;
	};

	// Oggバッチ再生
	// ファイルからOggを読んでWaveに変換
	class AOggBatch : public ABuffer {
		private:
			ByteBuff	_buff;

			friend class cereal::access;
			template <class Ar>
			friend void serialize(Ar&, AOggBatch&);
			AOggBatch() = default;
		public:
			AOggBatch(const HRW& hRW);
			bool isStreaming() const override;
			std::pair<const void*, std::size_t> getAllData(uint64_t offset) const override;
	};
	// Oggストリーミング再生
	// ファイルから読み込んでストリーミング
	class AOggStream : public ABuffer {
		private:
			VorbisFile	_vfile;
			mutable uint64_t 	_prevOffset;

			friend class cereal::access;
			template <class Ar>
			friend void serialize(Ar&, AOggStream&);
			AOggStream() = default;
		public:
			AOggStream(const HRW& hRW);
			bool isStreaming() const override;
			std::size_t getData(void* dst, uint64_t offset, std::size_t buffLen) const override;
			void invalidate() noexcept override;
	};
	class ASource : public Resource {
		private:
			struct FadeCB {
				virtual ~FadeCB() {}
				virtual void onFadeEnd(ASource& s) = 0;
			};
			template <class State>
			struct FadeCB_State : FadeCB {
				void onFadeEnd(ASource& s) override {
					s._setState<State>();
				}
			};
			struct IState {
				virtual ~IState() {}
				virtual void play(ASource& /*self*/, Duration /*fadeIn*/) {}
				virtual void pause(ASource& /*self*/, Duration /*fadeOut*/) {}
				virtual void stop(ASource& /*self*/, Duration /*fadeOut*/) {}
				virtual void update(ASource& /*self*/) {}
				virtual void timeSeek(ASource& /*self*/, Duration /*t*/) {}
				virtual void pcmSeek(ASource& /*self*/, int64_t /*t*/) {}
				virtual void onEnter(ASource& /*self*/, AState /*prev*/) {}
				virtual void onExit(ASource& /*self*/, AState /*next*/) {}
				virtual void setFadeTo(ASource& /*self*/, float /*gain*/, Duration /*dur*/) {}
				virtual void setBuffer(ASource& /*self*/, const HAb& /*hAb*/, uint32_t /*nLoop*/) {}
				virtual void sys_pause(ASource& /*self*/) {}
				virtual void sys_resume(ASource& /*self*/) {}
				virtual AState getState() const = 0;
			};

			//! 再生する音声データを持っていない
			struct S_Empty : IState {
				S_Empty(ASource& s);
				void onEnter(ASource& self, AState prev) override;
				void setBuffer(ASource& self, const HAb& hAb, uint32_t nLoop) override;
				AState getState() const override;
			};
			//! 音声データを持っているが、バッファにはまだ読み込まれてない
			struct S_Initial : IState {
				HAb			_hAb;
				uint32_t	_nLoop;

				S_Initial(ASource& s);
				S_Initial(ASource& s, const HAb& hAb, uint32_t nLoop);
				void _init();
				void onEnter(ASource& self, AState prev) override;
				void play(ASource& self, Duration fadeIn) override;
				void timeSeek(ASource& self, Duration t) override;
				void pcmSeek(ASource& self, int64_t t) override;
				void setBuffer(ASource& self, const HAb& hAb, uint32_t nLoop) override;
				AState getState() const override;
			};
			//! 再生中
			struct S_Playing : IState {
				Duration	_fadeIn;
				bool		_bSysPause;

				template <class State>
				void _fadeOut(ASource& self, Duration fadeOut);

				S_Playing(ASource& s, Duration fadeIn);
				void play(ASource& self, Duration fadeIn) override;
				void pause(ASource& self, Duration fadeOut) override;
				void stop(ASource& self, Duration fadeOut) override;
				void update(ASource& self) override;
				void timeSeek(ASource& self, Duration t) override;
				void pcmSeek(ASource& self, int64_t t) override;
				void onEnter(ASource& self, AState prev) override;
				void onExit(ASource& self, AState next) override;
				void setFadeTo(ASource& self, float gain, Duration dur) override;
				void setBuffer(ASource& self, const HAb& hAb, uint32_t nLoop) override;
				void sys_pause(ASource& self) override;
				void sys_resume(ASource& self) override;
				AState getState() const override;
			};
			//! 再生の一時停止。バッファもそのまま
			struct S_Paused : IState {
				S_Paused(ASource& s);
				void onEnter(ASource& self, AState prev) override;
				void play(ASource& self, Duration fadeIn) override;
				void stop(ASource& self, Duration fadeOut) override;
				void timeSeek(ASource& self, Duration t) override;
				void pcmSeek(ASource& self, int64_t t) override;
				void setBuffer(ASource& self, const HAb& hAb, uint32_t nLoop) override;
				AState getState() const override;
			};

			using UPState = std::unique_ptr<IState>;
			using OPBuf = spi::Optional<ABufSub>;
			HAb			_hAb;			//!< AudioAPIのバッファ管理
			UPState		_state,			//!< サウンド再生状態
						_nextState;
			OPBuf		_opBuf;			//!< サンプル読み出しを担う抽象クラス
			ASourceDep	_dep;			//!< 実際にAudioAPI呼び出しを行う環境依存クラス
			Duration	_duration;		//!< nLoopを考慮した場合の曲の長さ
			Timepoint	_tmUpdate;		//!< 前回Updateを呼んだ時刻
			uint32_t	_nLoop;
			float		_currentGain,
						_targetGain;
			Duration	_fadeInTime,
						_fadeOutTime;
			Duration	_timePos;		//!< 時間単位の再生位置
			uint64_t	_pcmPos;		//!< サンプル単位の再生位置
			uint32_t	_playedCur;		//!< 再生済みブロック数

			using UPFadeCB = std::unique_ptr<FadeCB>;
		public:
			struct Fade {
				Duration	durBegin, durEnd;
				float		fromGain, toGain;
				//! フェード処理が終わった時に呼ばれるコールバック
				/*! 通常、FADE_CHANGEの物しか使われない */
				UPFadeCB	callback;

				template <class Ar>
				friend void serialize(Ar&, Fade&);

				Fade();
				Fade(Fade&& f) = default;
				Fade& operator = (Fade&& f) = default;
				void init(Duration cur, Duration dur, float from, float to, FadeCB* cb=nullptr);
				//! 現在のボリュームを計算
				float calcGain(ASource& self);
			};
		private:
			enum FadeType {
				FADE_BEGIN,		//!< 曲初めのフェードイン
				FADE_END,		//!< 曲終わりのフェードアウト
				FADE_CHANGE,	//!< 曲途中の音量変更
				NUM_FADE
			};
			Fade		_fade[NUM_FADE];
		public:
			struct Save {
				HAb			hAb;
				int			stateID;
				uint32_t	nLoop;
				float		currentGain,
							targetGain;
				Duration	fadeInTime,
							fadeOutTime,
							timePos;

				template <class Ar>
				friend void serialize(Ar&, Save&);
				void pack(const ASource& self);
				void unpack(ASource& self);
			};
		private:
			template <class Ar>
			friend void save(Ar&, const ASource&);
			template <class Ar>
			friend void load(Ar&, ASource&);

			float _applyFades(float gain);
			void _advanceGain();
			void _refillBuffer(bool bClear);
			void _timeSeek(Duration t);
			void _pcmSeek(int64_t t);

			//! ステート切り替え処理
			template <class S, class... Ts>
			void _setState(Ts&&... ts) {
				D_Assert0(!_nextState);
				_nextState.reset(new S(*this, std::forward<Ts>(ts)...));
			}
			void _doChangeState();

		public:
			ASource();
			ASource(const ASource&) = delete;
			ASource(ASource&& s) = default;
			~ASource();

			const static Duration cs_zeroDur;
			void play(Duration fadeInTime=cs_zeroDur);
			void pause(Duration fadeOutTime=cs_zeroDur);
			void stop(Duration fadeOutTime=cs_zeroDur);
			//! ストリーミングキューの更新など
			void update();
			//! 任意の音量に変更
			/*!	\param[in] gain 目標音量
				\param[in] dur 遷移時間 */
			void setFadeTo(float gain, Duration dur);
			//! 曲初めのフェードイン指定
			void setFadeIn(Duration dur);
			//! 曲終わりのフェードアウト指定
			/*! 曲の残り時間がmsOutに満たない場合はそれに合わせて修正される */
			void setFadeOut(Duration dur);

			void setBuffer(const HAb& hAb, uint32_t nLoop=0);
			int getLooping() const;
			int getNLoop() const;
			void setPitch(float pitch);
			void setGain(float gain);
			//! このまま再生を続けた場合に曲が終了する時間 (ループ, ピッチ込み)
			Timepoint getEndTime();

			void setRelativeMode(bool bRel);
			void setPosition(const Vec3& pos);
			void setDirection(const Vec3& dir);
			void setVelocity(const Vec3& vel);
			void setGainRange(float gmin, float gmax);
			void setAngleGain(float inner, float outer);
			void setAngleOuterGain(float gain);

			Duration timeTell();
			uint64_t pcmTell();
			void timeSeek(Duration t);
			void pcmSeek(uint64_t p);
			AState getState() const;

			void sys_pause();
			void sys_resume();
			void invalidate();
			const char* getResourceName() const noexcept override;
	};
	class SSrcMgr : public spi::ResMgr<ASource>, public spi::Singleton<SSrcMgr> {};

	class SGroupMgr;
	//! ASourceをひとまとめにして管理
	class AGroup : public Resource {
		private:
			using SourceL = std::vector<HSs>;
			SourceL		_source;
			int			_nActive;
			bool		_bPaused;

			template <class Ar>
			friend void serialize(Ar&, AGroup&);
			friend class cereal::access;

			AGroup() = default;
		public:
			AGroup(int n);
			AGroup(const AGroup& a) = delete;
			AGroup(AGroup&& a) = default;
			void update();
			void pause();
			void resume();
			void clear();
			HSs play(const HAb& hAb, int nLoop=0);
			HSs fadeIn(const HAb& hAb, Duration fadeIn, int nLoop=0);
			HSs fadeInOut(const HAb& hAb, Duration fadeIn, Duration fadeOut, int nLoop=0);
			int getChannels() const;
			int getIdleChannels() const;
			int getPlayingChannels() const;
			void invalidate();
			const char* getResourceName() const noexcept override;
	};
	class SGroupMgr : public spi::ResMgr<AGroup>, public spi::Singleton<SGroupMgr> {};

	#define mgr_sound static_cast<SoundMgr&>(::rev::SoundMgrDep::ref())
	class SoundMgr : public SoundMgrDep {
		private:
			ABufMgr		_buffMgr;
			SSrcMgr 	_srcMgr;
			SGroupMgr	_sgMgr;

			template <class Ar>
			friend void serialize(Ar&, SoundMgr&);
			template <class T>
			friend struct cereal::LoadAndConstruct;

		public:
			using SoundMgrDep::SoundMgrDep;
			SoundMgr(const SoundMgr&) = delete;
			HAb loadWaveBatch(const std::string& name);
			HAb loadOggBatch(const std::string& name);
			HAb loadOggStream(const std::string& name);

			HSg createSourceGroup(int n);
			HSs createSource();
			void update();
			void pauseAllSound();
			void resumeAllSound();
			void invalidate();
	};
}
#include "../lua/import.hpp"
DEF_LUAIMPORT(rev::ABuffer)
DEF_LUAIMPORT(rev::AGroup)
DEF_LUAIMPORT(rev::ASource)
DEF_LUAIMPORT(rev::SoundMgr)
