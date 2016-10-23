#include "sound_common.hpp"

namespace rev {
	// --------------------- RawData ---------------------
	RawData::RawData(const std::size_t sz):
		buff(sz)
	{}
	RawData::RawData(RawData&& rd) noexcept:
		format(rd.format),
		buff(std::move(rd.buff))
	{}

	// --------------------- AFormat ---------------------
	AFormat::AFormat(const Format fmt) noexcept:
		format(fmt)
	{}
	AFormat::AFormat(const SDLAFormat f, const bool bStereo) noexcept:
		AFormat(f.getBitSize()>8, bStereo)
	{}
	AFormat::AFormat(const bool b16Bit, const bool bStereo) noexcept {
		if(b16Bit)
			format = bStereo ? Format::Stereo16 : Format::Mono16;
		else
			format = bStereo ? Format::Stereo8 : Format::Mono8;
	}
	int AFormat::getBitNum() const noexcept {
		if(format==Format::Mono8 || format==Format::Stereo8)
			return 8;
		return 16;
	}
	int AFormat::getChannels() const noexcept {
		if(format==Format::Mono8 || format==Format::Mono16)
			return 1;
		return 2;
	}
	std::size_t AFormat::getBlockSize() const noexcept {
		return getBitNum()/8 * getChannels();
	}
	// --------------------- AFormatF ---------------------
	AFormatF::AFormatF(const AFormat fmt, const int fr) noexcept:
		AFormat(fmt),
		freq(fr)
	{}

	// --------------------- SDLAFormat ---------------------
	SDLAFormat::SDLAFormat(const SDL_AudioFormat fmt) noexcept:
		format(fmt)
	{}
	SDLAFormat::SDLAFormat(const AFormat& afmt) noexcept:
		SDLAFormat(1, 0, 0, afmt.getBitNum())
	{}
	SDLAFormat::SDLAFormat(const int issigned, const int isbig, const int isfloat, const int bitsize) noexcept {
		format = bitsize & SDL_AUDIO_MASK_BITSIZE;
		format |= (isfloat & 1) << 8;
		format |= (isbig & 1) << 12;
		format |= (issigned & 1) << 15;
	}
	bool SDLAFormat::isSigned() const noexcept {
		return SDL_AUDIO_ISSIGNED(format);
	}
	bool SDLAFormat::isBigEndian() const noexcept {
		return SDL_AUDIO_ISBIGENDIAN(format);
	}
	bool SDLAFormat::isFloat() const noexcept {
		return SDL_AUDIO_ISFLOAT(format);
	}
	std::size_t SDLAFormat::getBitSize() const noexcept {
		return SDL_AUDIO_BITSIZE(format);
	}

	// --------------------- SDLAFormatCF ---------------------
	SDLAFormatCF::SDLAFormatCF(const SDLAFormat fmt, const int fr) noexcept:
		SDLAFormat(fmt),
		channels(fmt.getBitSize()==8 ? 1 : 2),
		freq(fr)
	{}
	SDLAFormatCF::SDLAFormatCF(const AFormatF& af) noexcept:
		SDLAFormat(static_cast<AFormat>(af)),
		channels(af.getChannels()),
		freq(af.freq)
	{}
}
