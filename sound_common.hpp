#pragma once
#include "abstbuffer.hpp"
#include "handle.hpp"
#include <SDL_audio.h>
#include <vorbis/vorbisfile.h>
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>

namespace rev {
	enum class AState {
		Initial,
		Playing,
		Paused,
		Stopped,
		Empty
	};
	struct AFormat;
	struct AFormatF;
	struct SDLAFormat {
		SDL_AudioFormat format;

		template <class Ar>
		SDL_AudioFormat save_minimal(const Ar&) const {
			return format;
		}
		template <class Ar>
		void load_minimal(const Ar&, const SDL_AudioFormat& f) {
			format = f;
		}

		SDLAFormat(SDL_AudioFormat fmt) noexcept;
		SDLAFormat(int issigned, int isbig, int isfloat, int bitsize) noexcept;
		SDLAFormat(const AFormat& afmt) noexcept;
		bool isSigned() const noexcept;
		bool isBigEndian() const noexcept;
		bool isFloat() const noexcept;
		std::size_t getBitSize() const noexcept;
	};
	struct SDLAFormatCF : SDLAFormat {
		int channels,
			freq;

		template <class Ar>
		void serialize(Ar& ar) {
			ar(cereal::base_class<SDLAFormat>(this), channels, freq);
		}

		SDLAFormatCF() = default;
		SDLAFormatCF(SDLAFormat fmt, int fr) noexcept;
		SDLAFormatCF(const AFormatF& af) noexcept;
	};

	struct AFormat {
		enum class Format : int {
			Mono8,
			Mono16,
			Stereo8,
			Stereo16,
			Invalid
		};
		Format	format;

		template <class Ar>
		int save_minimal(const Ar&) const {
			return static_cast<int>(format);
		}
		template <class Ar>
		void load_minimal(const Ar&, const int& f) {
			format = static_cast<Format>(f);
		}

		AFormat(Format fmt=Format::Invalid) noexcept;
		AFormat(SDLAFormat f, bool bStereo) noexcept;
		AFormat(bool b16Bit, bool bStereo) noexcept;
		int getBitNum() const noexcept;
		int getChannels() const noexcept;
		std::size_t getBlockSize() const noexcept;
	};
	struct AFormatF : AFormat {
		int		freq;

		template <class Ar>
		void serialize(Ar& ar) {
			ar(cereal::base_class<AFormat>(this), freq);
		}

		AFormatF() = default;
		AFormatF(AFormat fmt, int fr) noexcept;
	};
	struct RawData {
		AFormatF	format;
		ByteBuff	buff;

		RawData() = default;
		RawData(std::size_t sz);
		RawData(RawData&& d) noexcept;
	};
	constexpr static int MAX_AUDIO_BLOCKNUM = 4;
	extern ov_callbacks OVCallbacksNF, OVCallbacks;
	// VorbisFile wrapper
	class VorbisFile {
		private:
			int64_t			_initialFPos;
			HRW				_hRW;
			OggVorbis_File	_ovf;
			AFormatF		_format;
			double			_dTotal;
			int64_t			_iTotal;

			friend class AOggStream;
			friend class cereal::access;
			template <class Ar>
			void load(Ar& ar) {
				ar(_hRW, _initialFPos);
				_init();
				int64_t fpos;
				ar(fpos);
				pcmSeek(fpos);
			}
			template <class Ar>
			void save(Ar& ar) const {
				ar(_hRW, _initialFPos);
				int64_t fpos = pcmTell();
				ar(fpos);
			}
			void _init();
			VorbisFile() = default;

		public:
			static std::size_t ReadOGC(void* ptr, std::size_t blocksize, std::size_t nmblock, void* datasource) noexcept;
			static int SeekOGC(void* datasource, ogg_int64_t offset, int whence) noexcept;
			static int CloseOGC(void* datasource) noexcept;
			static long TellOGC(void* datasource) noexcept;
			static ov_callbacks OVCallbacksNF,
			OVCallbacks;

			VorbisFile(const HRW& hRW) noexcept;
			~VorbisFile();
			const AFormatF& getFormat() const noexcept;
			//! 一括読み出し
			static RawData ReadAll(HRW hRW);
			//! 指定サイズのデータを読み出し
			std::size_t read(void* dst, std::size_t toRead);
			bool isEOF() const NOEXCEPT_IF_RELEASE;

			bool timeSeek(double s) NOEXCEPT_IF_RELEASE;
			void timeSeekPage(double s) NOEXCEPT_IF_RELEASE;
			bool timeSeekLap(double s) NOEXCEPT_IF_RELEASE;
			void timeSeekPageLap(double s) NOEXCEPT_IF_RELEASE;
			bool pcmSeek(int64_t pos) NOEXCEPT_IF_RELEASE;
			void pcmSeekPage(int64_t pos) NOEXCEPT_IF_RELEASE;
			bool pcmSeekLap(int64_t pos) NOEXCEPT_IF_RELEASE;
			void pcmSeekPageLap(int64_t pos) NOEXCEPT_IF_RELEASE;
			double timeLength() const noexcept;
			int64_t pcmLength() const noexcept;

			double timeTell() const NOEXCEPT_IF_RELEASE;
			int64_t pcmTell() const NOEXCEPT_IF_RELEASE;

			//! 内包リソースハンドルをリリースせず無効化 (=以後使用不可)
			void invalidate() noexcept;
	};
}
namespace cereal {
	template <class Ar>
	struct specialize<Ar, ::rev::SDLAFormatCF, cereal::specialization::member_serialize> {};
	template <class Ar>
	struct specialize<Ar, ::rev::AFormatF, cereal::specialization::member_serialize> {};
}
