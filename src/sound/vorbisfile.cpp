#include "common.hpp"
#include "../sdl_rw.hpp"
#include "error.hpp"

namespace rev {
	std::size_t VorbisFile::ReadOGC(void* ptr, const std::size_t blocksize, const std::size_t nmblock, void* datasource) noexcept {
		auto* ops = reinterpret_cast<SDL_RWops*>(datasource);
		const auto fpos = SDL_RWseek(ops, 0, RW_SEEK_CUR);
		const auto fsize = SDL_RWseek(ops, 0, RW_SEEK_END);
		SDL_RWseek(ops, fpos, RW_SEEK_SET);
		const std::size_t nblock = std::min((fsize-fpos)/static_cast<decltype(fpos)>(blocksize), static_cast<decltype(fpos)>(nmblock));
		return SDL_RWread(ops, ptr, blocksize, nblock);
	}
	int VorbisFile::SeekOGC(void* datasource, ogg_int64_t offset, int whence) noexcept {
		auto* ops = reinterpret_cast<SDL_RWops*>(datasource);
		if(!ops)
			return -1;
		if(whence == SEEK_SET)
			whence = RW_SEEK_SET;
		else if(whence == SEEK_CUR)
			whence = RW_SEEK_CUR;
		else if(whence == SEEK_END)
			whence = RW_SEEK_END;
		return SDL_RWseek(ops, offset, whence);
	}
	int VorbisFile::CloseOGC(void* datasource) noexcept {
		auto* ops = reinterpret_cast<SDL_RWops*>(datasource);
		if(!ops)
			return EOF;
		SDL_RWclose(ops);
		return 0;
	}
	long VorbisFile::TellOGC(void* datasource) noexcept {
		auto* ops = reinterpret_cast<SDL_RWops*>(datasource);
		if(!ops)
			return -1;
		return SDL_RWtell(ops);
	}

	ov_callbacks VorbisFile::OVCallbacksNF = {
		ReadOGC, SeekOGC, nullptr, TellOGC
	};
	ov_callbacks VorbisFile::OVCallbacks = {
		ReadOGC, SeekOGC, CloseOGC, TellOGC
	};
	RawData VorbisFile::ReadAll(HRW hRW) {
		VorbisFile vf(hRW);
		const auto totalbyte = vf.getFormat().getBlockSize() * vf.pcmLength();
		RawData rd(totalbyte);
		const std::size_t nread = vf.read(rd.buff.data(), totalbyte);
		rd.buff.resize(nread);
		rd.format = vf.getFormat();
		return rd;
	}
	void VorbisFile::_init() {
		auto& rw = *_hRW;
		rw.seek(_initialFPos, RWops::Pos::Begin);
		D_OVAssert(ov_open_callbacks, rw.getOps(), &_ovf, nullptr, 0, OVCallbacksNF);
		vorbis_info* info = ov_info(&_ovf, -1);
		// Oggのフォーマットは全てsigned int 16bitとみなす
		_format = AFormatF(AFormat(true, info->channels > 1), info->rate);
		_dTotal = D_OVAssert(ov_time_total, &_ovf, -1);
		_iTotal = D_OVAssert(ov_pcm_total, &_ovf, -1);
	}
	VorbisFile::VorbisFile(const HRW& hRW) noexcept {
		_hRW = hRW;
		_initialFPos = hRW->tell();
		_init();
	}
	VorbisFile::~VorbisFile() {
		D_OVAssert(ov_clear, &_ovf);
	}
	const AFormatF& VorbisFile::getFormat() const noexcept {
		return _format;
	}
	bool VorbisFile::isEOF() const NOEXCEPT_IF_RELEASE {
		return pcmTell() == _iTotal;
	}
	bool VorbisFile::timeSeek(const double s) NOEXCEPT_IF_RELEASE {
		D_OVAssert(ov_time_seek, &_ovf, s);
		return isEOF();
	}
	void VorbisFile::timeSeekPage(const double s) NOEXCEPT_IF_RELEASE {
		D_OVAssert(ov_time_seek_page, &_ovf, s);
	}
	bool VorbisFile::timeSeekLap(const double s) NOEXCEPT_IF_RELEASE {
		D_OVAssert(ov_time_seek_lap, &_ovf, s);
		return isEOF();
	}
	void VorbisFile::timeSeekPageLap(const double s) NOEXCEPT_IF_RELEASE {
		D_OVAssert(ov_time_seek_page_lap, &_ovf, s);
	}
	bool VorbisFile::pcmSeek(const int64_t pos) NOEXCEPT_IF_RELEASE {
		D_OVAssert(ov_pcm_seek, &_ovf, pos);
		return isEOF();
	}
	void VorbisFile::pcmSeekPage(const int64_t pos) NOEXCEPT_IF_RELEASE {
		D_OVAssert(ov_pcm_seek_page, &_ovf, pos);
	}
	bool VorbisFile::pcmSeekLap(const int64_t pos) NOEXCEPT_IF_RELEASE {
		D_OVAssert(ov_pcm_seek_lap, &_ovf, pos);
		return isEOF();
	}
	void VorbisFile::pcmSeekPageLap(const int64_t pos) NOEXCEPT_IF_RELEASE {
		D_OVAssert(ov_pcm_seek_page_lap, &_ovf, pos);
	}
	std::size_t VorbisFile::read(void* dst, std::size_t toRead) {
		if(toRead == 0)
			return 0;

		auto pDst = reinterpret_cast<char*>(dst);
		std::size_t nRead;
		int bs;
		do {
			nRead = ov_read(&_ovf, pDst, toRead, 0, 2, 1, &bs);
			if(nRead == 0)
				break;
			toRead -= nRead;
			pDst += nRead;
		} while(toRead > 0);
		return pDst - reinterpret_cast<char*>(dst);
	}
	double VorbisFile::timeLength() const noexcept {
		return _dTotal;
	}
	int64_t VorbisFile::pcmLength() const noexcept {
		return _iTotal;
	}
	double VorbisFile::timeTell() const NOEXCEPT_IF_RELEASE {
		return D_OVAssert(ov_time_tell, const_cast<OggVorbis_File*>(&this->_ovf));
	}
	int64_t VorbisFile::pcmTell() const NOEXCEPT_IF_RELEASE {
		return D_OVAssert(ov_pcm_tell, const_cast<OggVorbis_File*>(&this->_ovf));
	}
	void VorbisFile::invalidate() noexcept {
		_hRW.reset();
	}
}
