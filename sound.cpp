#include "sound.hpp"
#include "sound_common.hpp"

namespace rev {
	Duration CalcTimeLength(const int word_size, const int ch, const int hz, const std::size_t buffLen) noexcept {
		auto dur = double(buffLen) / double(word_size * ch * hz);
		return std::chrono::microseconds(static_cast<uint64_t>(dur * 1000000));
	}
	uint64_t CalcSampleLength(int word_size, int ch, int hz, Duration dur) noexcept {
		using namespace std::chrono;
		auto bsize = word_size * ch;
		auto dd = static_cast<double>(duration_cast<microseconds>(dur).count()) / 1000000.0;
		dd *= double(bsize * hz);
		// キリの良い値に切り下げる
		return static_cast<uint64_t>(dd) / bsize * bsize;
	}
	uint64_t CalcSampleLength(const AFormatF& af, Duration dur) noexcept {
		return CalcSampleLength(af.getBitNum()/8, af.getChannels(), af.freq, dur);
	}

	// --------------------- ABufMgr ---------------------
	const std::string ABufMgr::cs_rtname[] = {
		"sound"
	};
	ABufMgr::ABufMgr():
		ResMgrApp(cs_rtname)
	{}
	// --------------------- ABuffer ---------------------
	ABuffer::ABuffer():
		_format(AFormat::Format::Invalid, 0)
	{}
	const char* ABuffer::getResourceName() const noexcept {
		return ::rev::resource_name::ABuffer;
	}
	// --------------------- ABufSub ---------------------
	ABufSub::ABufSub(const HAb& hAb, const uint32_t nLoop):
		_hAb(hAb),
		_nLoop(nLoop),
		_nLoopInit(nLoop)
	{
		if(hAb->isStreaming()) {
			_abuff.reset(new ABufferDep[MAX_AUDIO_BLOCKNUM]);
			_nBuffer = MAX_AUDIO_BLOCKNUM;
		} else {
			_abuff.reset(new ABufferDep[1]);
			_nBuffer = 1;
		}
		_offset = 0;
		_readCur = _playedCur = _writeCur = 0;
	}
	const AFormatF& ABufSub::getFormat() const {
		return _hAb->getFormat();
	}
	Duration ABufSub::getDuration() const {
		return _hAb->getDuration();
	}
	void ABufSub::_fillBuffer() {
		auto& ab = *_hAb;
		if(ab.isStreaming()) {
			constexpr int BUFFERSIZE = 8192*4;
			uint8_t buff[BUFFERSIZE];
			while(_writeCur < _playedCur + _nBuffer) {
				auto nread = ab.getData(buff, _offset, sizeof(buff));
				if(nread == 0) {
					if(_nLoop == 0)
						break;
					--_nLoop;
					_offset = 0;
				}
				_offset += nread;
				_abuff[_writeCur % _nBuffer].writeBuffer(ab.getFormat(), buff, nread);
				++_writeCur;
			}
		} else {
			if(_writeCur != 1) {
				auto res = ab.getAllData(_offset);
				_abuff[0].writeBuffer(ab.getFormat(), res.first, res.second);
				++_writeCur;
			}
		}
	}
	ABufferDep* ABufSub::getBlock() {
		_fillBuffer();
		if(_readCur == _writeCur)
			return nullptr;
		ABufferDep* ret = &_abuff[_readCur % _nBuffer];
		++_readCur;
		return ret;
	}
	void ABufSub::rewind() {
		_readCur = _writeCur = _playedCur = 0;
		_offset = 0;
		_nLoop = _nLoopInit;
	}
	bool ABufSub::isEOF() {
		_fillBuffer();
		return _playedCur == _writeCur;
	}
	void ABufSub::setPlayedCursor(const int cur) {
		D_Assert0(_playedCur <= cur);
		D_Assert0(_writeCur >= cur);
		_playedCur = cur;
	}
	void ABufSub::timeSeek(const Duration t) {
		_readCur = _writeCur = _playedCur = 0;
		const AFormatF& af = _hAb->getFormat();
		_offset = CalcSampleLength(af, t);
	}
	void ABufSub::pcmSeek(const uint64_t t) {
		_readCur = _writeCur = _playedCur = 0;
		const AFormatF& af = _hAb->getFormat();
		_offset = af.getBlockSize() * t;
	}
	void ABufSub::invalidate() {
		_hAb->invalidate();
		_hAb.reset();
		_abuff.reset();
	}

	// --------------------- AWaveBatch ---------------------
	namespace {
		struct SDLMem {
			Uint8* ptr = nullptr;
			~SDLMem() {
				if(ptr)
					SDL_free(ptr);
			}
		};
	}
	AWaveBatch::AWaveBatch(const HRW& hRW) {
		SDL_AudioSpec spec;
		SDLMem buff;
		Uint32 buffLen;
		SDLAssert(SDL_LoadWAV_RW, hRW->getOps(), 0, &spec, &buff.ptr, &buffLen);
		AFormat fmt(SDL_AUDIO_BITSIZE(spec.format) > 8, spec.channels!=1);
		_format = AFormatF(fmt, spec.freq);
		_buff.resize(buffLen);
		std::memcpy(&_buff[0], buff.ptr, buffLen);
		_duration = CalcTimeLength(fmt.getBitNum()/8, fmt.getChannels(), _format.freq, buffLen);
	}
	bool AWaveBatch::isStreaming() const {
		return false;
	}
	std::pair<const void*, std::size_t> AWaveBatch::getAllData(const uint64_t offset) const {
		return std::make_pair(&_buff[offset], _buff.size()-offset);
	}

	// --------------------- AOggBatch ---------------------
	AOggBatch::AOggBatch(const HRW& hRW) {
		RawData rd = VorbisFile::ReadAll(hRW);
		_format = rd.format;
		_duration = CalcTimeLength(_format.getBitNum()/8, _format.getChannels(), _format.freq, rd.buff.size());
		_buff = std::move(rd.buff);
	}
	bool AOggBatch::isStreaming() const {
		return false;
	}
	std::pair<const void*, std::size_t> AOggBatch::getAllData(const uint64_t offset) const {
		return std::make_pair(&_buff[offset], _buff.size()-offset);
	}

	// --------------------- AOggStream ---------------------
	AOggStream::AOggStream(const HRW& hRW):
		_vfile(hRW)
	{
		_format = _vfile.getFormat();
		_prevOffset = ~0;
		_duration = std::chrono::milliseconds(static_cast<uint32_t>(_vfile.timeLength() * 1000.f));
	}
	bool AOggStream::isStreaming() const {
		return true;
	}
	std::size_t AOggStream::getData(void* dst, const uint64_t offset, const std::size_t buffLen) const {
		auto& vfile = const_cast<VorbisFile&>(_vfile);
		const uint64_t bsize = _format.getBlockSize();
		Assert0(offset % bsize == 0);
		if(_prevOffset != offset) {
			auto pcmOffset = offset / bsize;
			vfile.pcmSeek(pcmOffset);
		}
		const std::size_t nread = vfile.read(dst, buffLen);
		_prevOffset = offset + nread;
		return nread;
	}
	void AOggStream::invalidate() noexcept {
		_vfile.invalidate();
	}

	// --------------------- ASource::Fade ---------------------
	ASource::Fade::Fade():
		durBegin(cs_zeroDur),
		durEnd(cs_zeroDur),
		fromGain(0),
		toGain(0)
	{}
	void ASource::Fade::init(const Duration cur, const Duration dur, const float from, const float to, FadeCB* cb) {
		durBegin =cur;
		durEnd = cur+dur;
		fromGain = from;
		toGain = to;
		callback.reset(cb);
	}
	float ASource::Fade::calcGain(ASource& self) {
		const Duration cur = self._timePos;
		if(cur >= durEnd) {
			if(callback) {
				callback->onFadeEnd(self);
				callback.reset(nullptr);
			}
			return toGain;
		}
		if(cur <= durBegin)
			return fromGain;
		using namespace std::chrono;
		const float r = float(duration_cast<microseconds>(cur - durBegin).count()) / float(duration_cast<microseconds>(durEnd - durBegin).count());
		return (toGain-fromGain) * r + fromGain;
	}

	// --------------------- ASource::Save ---------------------
	void ASource::Save::pack(const ASource& self) {
		hAb = self._hAb;
		stateID = static_cast<int>(self._state->getState());
		nLoop = self._nLoop;

		currentGain = self._currentGain;
		targetGain = self._targetGain;
		fadeInTime = self._fadeInTime;
		fadeOutTime = self._fadeOutTime;
		timePos = self._timePos;
	}
	void ASource::Save::unpack(ASource& self) {
		// _tmUpdateはロードした時刻に合わせる
		self._tmUpdate = Clock::now();
		if(hAb)
			self.setBuffer(hAb, nLoop);

		self._currentGain = currentGain;
		self._targetGain = targetGain;
		self._fadeInTime = fadeInTime;
		self._fadeOutTime = fadeOutTime;

		switch(static_cast<AState>(stateID)) {
			case AState::Initial:
				std::cout << "RESTORE Initial" << std::endl;
				break;
			case AState::Playing:
				std::cout << "RESTORE Playing" << std::endl;
				self.timeSeek(timePos);
				self.play();
				break;
			case AState::Paused:
				std::cout << "RESTORE Paused" << std::endl;
				self.timeSeek(timePos);
				break;
			case AState::Stopped:
				std::cout << "RESTORE Stopped" << std::endl;
				break;
			case AState::Empty:
				std::cout << "RESTORE Empty" << std::endl;
				break;
			default:
				D_Assert0(false);
				break;
		}
	}

	// --------------------- ASource ---------------------
	const Duration ASource::cs_zeroDur = std::chrono::seconds(0);
	ASource::ASource():
		_state(new S_Empty(*this))
	{
		_fadeInTime = _fadeOutTime = cs_zeroDur;
		auto& fdBeg = _fade[FADE_BEGIN];
		auto& fdEnd = _fade[FADE_END];
		auto& fdChg = _fade[FADE_CHANGE];
		fdBeg.fromGain = 0;
		fdBeg.toGain = 1;
		fdEnd.fromGain = 1;
		fdEnd.toGain = 0;
		fdChg.fromGain = fdChg.toGain = 1;
	}
	ASource::~ASource() {
		if(_state)
			stop();
	}
	void ASource::_timeSeek(const Duration t) {
		if(t >= _duration) {
			_timePos = _duration;
			return;
		}
		auto t2 = t;
		while(t2 >= _duration)
			t2 -= _duration;

		_opBuf->timeSeek(t2);
		_timePos = t;
		_pcmPos = CalcSampleLength(_opBuf->getFormat(), t);
		_playedCur = 0;
	}
	void ASource::_pcmSeek(int64_t t) {
		AFormatF af = _opBuf->getFormat();
		t = (af.getBlockSize() * af.freq * 1000000) / t;
		_timePos = std::chrono::microseconds(t);
		_timeSeek(_timePos);
	}
	void ASource::_doChangeState() {
		if(_nextState) {
			AState st = _state->getState(),
				nst = _nextState->getState();
			_state->onExit(*this, nst);
			_state.reset(nullptr);
			std::swap(_state, _nextState);
			_state->onEnter(*this, st);
		}
	}
	void ASource::play(const Duration fadeIn) {
		_state->play(*this, fadeIn);
		_doChangeState();
	}
	void ASource::pause(const Duration fadeOut) {
		_state->pause(*this, fadeOut);
		_doChangeState();
	}
	void ASource::stop(const Duration fadeOut) {
		_state->stop(*this, fadeOut);
		_doChangeState();
	}
	void ASource::update() {
		_state->update(*this);
		_doChangeState();
	}
	void ASource::setPitch(const float pitch) {
		_dep.setPitch(pitch);
	}
	void ASource::setGain(const float gain) {
		_targetGain = gain;
	}
	void ASource::timeSeek(const Duration t) {
		_state->timeSeek(*this, t);
		_doChangeState();
	}
	void ASource::setBuffer(const HAb& hAb, const uint32_t nLoop) {
		_state->setBuffer(*this, hAb, nLoop);
		_doChangeState();
	}
	AState ASource::getState() const {
		return _state->getState();
	}
	void ASource::setRelativeMode(const bool bRel) {
		_dep.setRelativeMode(bRel);
	}
	void ASource::setPosition(const Vec3& pos) {
		_dep.setPosition(pos);
	}
	void ASource::setDirection(const Vec3& dir) {
		_dep.setDirection(dir);
	}
	void ASource::setVelocity(const Vec3& vel) {
		_dep.setVelocity(vel);
	}
	void ASource::setGainRange(const float gmin, const float gmax) {
		_dep.setGainRange(gmin, gmax);
	}
	void ASource::setAngleGain(const float inner, const float outer) {
		_dep.setAngleGain(inner, outer);
	}
	void ASource::setAngleOuterGain(const float gain) {
		_dep.setAngleOuterGain(gain);
	}
	int ASource::getLooping() const {
		return _timePos / _duration;
	}
	int ASource::getNLoop() const {
		return _nLoop;
	}
	Duration ASource::timeTell() {
		return _timePos;
	}
	uint64_t ASource::pcmTell() {
		return _pcmPos;
	}
	void ASource::sys_pause() {
		_state->sys_pause(*this);
	}
	void ASource::sys_resume() {
		_state->sys_resume(*this);
	}

	void ASource::pcmSeek(const uint64_t p) {
		_state->pcmSeek(*this, p);
		_doChangeState();
	}
	void ASource::setFadeIn(const Duration dur) {
		_fadeInTime = dur;
	}
	void ASource::setFadeTo(const float gain, const Duration dur) {
		_state->setFadeTo(*this, gain, dur);
		_doChangeState();
	}
	void ASource::setFadeOut(const Duration dur) {
		_fadeOutTime = dur;
	}
	float ASource::_applyFades(float /*gain*/) {
		const float r = std::min(_fade[FADE_BEGIN].calcGain(*this),
								_fade[FADE_END].calcGain(*this));
		return std::min(r, _fade[FADE_CHANGE].calcGain(*this));
	}
	void ASource::_advanceGain() {
		_targetGain = _applyFades(_currentGain);
		_currentGain += (_targetGain - _currentGain)*0.75f;
		_dep.setGain(_currentGain);
	}
	void ASource::_refillBuffer(const bool bClear) {
		if(bClear)
			_dep.clearBlock();
		// 最初のキューの準備
		int num = MAX_AUDIO_BLOCKNUM;
		while(--num >= 0) {
			auto* pb = _opBuf->getBlock();
			if(!pb)
				break;
			_dep.enqueue(*pb);
		}
	}
	void ASource::invalidate() {
		stop();
		_hAb.reset();
		if(_opBuf) {
			_opBuf->invalidate();
			_opBuf = spi::none;
		}
	}
	const char* ASource::getResourceName() const noexcept {
		return ::rev::resource_name::ASource;
	}
	// --------------------- ASource::S_Empty ---------------------
	ASource::S_Empty::S_Empty(ASource& /*s*/) {}
	void ASource::S_Empty::onEnter(ASource& self, AState /*prev*/) {
		std::cout << "S_Empty" << std::endl;
		self._pcmPos = 0;
		self._timePos = std::chrono::seconds(0);
	}
	void ASource::S_Empty::setBuffer(ASource& self, const HAb& hAb, const uint32_t nLoop) {
		self._setState<S_Initial>(hAb, nLoop);
	}
	AState ASource::S_Empty::getState() const {
		return AState::Empty;
	}

	// --------------------- ASource::S_Initial ---------------------
	ASource::S_Initial::S_Initial(ASource& /*s*/, const HAb& hAb, const uint32_t nLoop):
		_hAb(hAb),
		_nLoop(nLoop)
	{}
	ASource::S_Initial::S_Initial(ASource& /*s*/) {}
	void ASource::S_Initial::onEnter(ASource& self, AState /*prev*/) {
		std::cout << "S_Initial" << std::endl;
		self._dep.reset();
		self._dep.clearBlock();

		D_Assert0(self._hAb || _hAb);
		if(_hAb) {
			self._hAb = _hAb;
			self._nLoop = _nLoop;
		}
		self._duration = self._hAb->getDuration() * (_nLoop+1);
		self._dep.setGain(0);
		self._opBuf = ABufSub(self._hAb, self._nLoop);

		self._pcmPos = 0;
		self._timePos = cs_zeroDur;
		self._playedCur = 0;
		self._opBuf->rewind();
		self._currentGain = self._targetGain = 0.f;

		Duration dur = (self._duration < self._fadeInTime) ?
						cs_zeroDur :
						self._fadeInTime;
		self._fade[FADE_BEGIN].init(cs_zeroDur, dur, 0.f, 1.f);
		dur = (self._duration < self._fadeOutTime) ?
				cs_zeroDur :
				(self._duration - self._fadeOutTime);
		self._fade[FADE_END].init(dur, self._duration, 1.f, 0.f);
		self._fade[FADE_CHANGE].init(cs_zeroDur, cs_zeroDur, 1.f, 1.f);
	}
	void ASource::S_Initial::play(ASource& self, const Duration fadeIn) {
		self._refillBuffer(true);
		self._setState<S_Playing>(fadeIn);
	}
	void ASource::S_Initial::timeSeek(ASource& self, const Duration t) {
		self._timeSeek(t);
	}
	void ASource::S_Initial::pcmSeek(ASource& self, const int64_t t) {
		self._pcmSeek(t);
	}
	void ASource::S_Initial::setBuffer(ASource& self, const HAb& hAb, const uint32_t nLoop) {
		self._setState<S_Initial>(hAb, nLoop);
	}
	AState ASource::S_Initial::getState() const {
		return AState::Initial;
	}

	// --------------------- ASource::S_Playing ---------------------
	ASource::S_Playing::S_Playing(ASource& /*s*/, const Duration fadeIn):
		_fadeIn(fadeIn),
		_bSysPause(false)
	{}
	void ASource::S_Playing::onEnter(ASource& self, AState /*prev*/) {
		std::cout << "S_Playing" << std::endl;

		self._tmUpdate = Clock::now();
		auto& fd = self._fade[FADE_CHANGE];
		fd.durBegin = self._timePos;
		fd.durEnd = self._timePos + _fadeIn;
		fd.fromGain = 0.f;
		fd.toGain = 1.f;

		update(self);
		self._dep.play();
	}
	void ASource::S_Playing::play(ASource& /*self*/, Duration /*fadeIn*/) {}
	template <class State>
	void ASource::S_Playing::_fadeOut(ASource& self, const Duration fadeOut) {
		if(fadeOut == cs_zeroDur)
			self._setState<State>();
		else {
			auto& fd = self._fade[FADE_CHANGE];
			fd.init(self._timePos, fadeOut, self._currentGain, 0.f, new FadeCB_State<State>());
		}
	}
	void ASource::S_Playing::pause(ASource& self, const Duration fadeOut) {
		_fadeOut<S_Paused>(self, fadeOut);
	}
	void ASource::S_Playing::stop(ASource& self, const Duration fadeOut) {
		_fadeOut<S_Initial>(self, fadeOut);
	}
	void ASource::S_Playing::update(ASource& self) {
		// 前回のUpdateを呼んだ時間からの差を累積
		Timepoint tnow = Clock::now();
		Duration diff = tnow - self._tmUpdate;
		self._tmUpdate = tnow;
		self._timePos = std::min(self._timePos + diff, self._duration);
		// 再生時間からサンプル数を計算
		const AFormatF& af = self._opBuf->getFormat();
		self._pcmPos = CalcSampleLength(af, self._timePos);
		// 終了チェック
		if(self._opBuf->isEOF()) {
			// ループ回数が残っていればもう一周
			if(self._timePos == self._duration) {
				// そうでなければ終了ステートへ
				self._setState<S_Initial>();
				return;
			} else {
				// 何もしない (ブロックの最後まで再生を待つ)
			}
		} else {
			// キューの更新
			int nproc = self._dep.getUsedBlock();
			if(nproc > 0) {
				self._playedCur += nproc;
				self._opBuf->setPlayedCursor(self._playedCur);
				ABufferDep* pb;
				while(--nproc >= 0 && (pb = self._opBuf->getBlock()))
					self._dep.enqueue(*pb);
			}
		}
		self._dep.update(!self._opBuf->isEOF());
		self._advanceGain();
	}
	void ASource::S_Playing::timeSeek(ASource& self, Duration t) {
		self._dep.reset();
		self._timeSeek(t);
		self._dep.play();
	}
	void ASource::S_Playing::pcmSeek(ASource& self, int64_t t) {
		self._dep.reset();
		self._pcmSeek(t);
		self._dep.play();
	}
	void ASource::S_Playing::onExit(ASource& self, AState /*next*/) {
		Duration dur = Clock::now() - self._tmUpdate;
		self._timePos += dur;
		self._fade[FADE_CHANGE].init(cs_zeroDur, cs_zeroDur, 1.f, 1.f);
	}
	void ASource::S_Playing::setFadeTo(ASource& self, const float gain, const Duration dur) {
		self._fade[FADE_CHANGE].init(self._timePos, dur, self._currentGain, gain);
	}
	void ASource::S_Playing::setBuffer(ASource& self, const HAb& hAb, const uint32_t nLoop) {
		self._setState<S_Initial>(hAb, nLoop);
	}
	void ASource::S_Playing::sys_pause(ASource& self) {
		self._dep.pause();
		self._timePos = self._timePos + (Clock::now() - self._tmUpdate);
		_bSysPause = true;
	}
	void ASource::S_Playing::sys_resume(ASource& self) {
		self._tmUpdate = Clock::now();
		if(_bSysPause) {
			_bSysPause = false;
			self._dep.play();
		}
	}
	AState ASource::S_Playing::getState() const {
		return AState::Playing;
	}

	// --------------------- ASource::S_Paused ---------------------
	ASource::S_Paused::S_Paused(ASource& /*s*/) {}
	void ASource::S_Paused::onEnter(ASource& self, AState /*prev*/) {
		std::cout << "S_Paused" << std::endl;
		self._dep.pause();
	}
	void ASource::S_Paused::play(ASource& self, const Duration fadeIn) {
		self._setState<S_Playing>(fadeIn);
	}
	void ASource::S_Paused::stop(ASource& self, Duration /*fadeOut*/) {
		self._setState<S_Initial>();
	}
	void ASource::S_Paused::timeSeek(ASource& self, const Duration t) {
		auto& self2 = self;
		self._setState<S_Initial>();
		self2.timeSeek(t);
	}
	void ASource::S_Paused::pcmSeek(ASource& self, const int64_t t) {
		auto& self2 = self;
		self._setState<S_Initial>();
		self2.pcmSeek(t);
	}
	void ASource::S_Paused::setBuffer(ASource& self, const HAb& hAb, const uint32_t nLoop) {
		self._setState<S_Initial>(hAb, nLoop);
	}
	AState ASource::S_Paused::getState() const {
		return AState::Paused;
	}

	// ------------------ ALGroup ------------------
	AGroup::AGroup(const int n):
		_source(n),
		_nActive(0),
		_bPaused(false)
	{
		for(auto& s : _source)
			s = mgr_sound.createSource();
	}
	void AGroup::update() {
		int na = 0;
		for(auto& s : _source) {
			auto st = s->getState();
			if(st == AState::Playing ||
				st == AState::Paused)
				++na;
		}
		_nActive = na;
	}
	void AGroup::pause() {
		if(!_bPaused) {
			_bPaused = true;
			for(auto& s : _source)
				s->pause();
		}
	}
	void AGroup::resume() {
		if(_bPaused) {
			_bPaused = false;
			for(auto& s : _source)
				s->play();
		}
	}
	void AGroup::clear() {
		for(auto& s : _source)
			s->stop();
		_nActive = 0;
		_bPaused = false;
	}
	HSs AGroup::play(const HAb& hAb, const int nLoop) {
		return fadeIn(hAb, std::chrono::seconds(0), nLoop);
	}
	HSs AGroup::fadeIn(const HAb& hAb, const Duration fadeIn, const int nLoop) {
		return fadeInOut(hAb, fadeIn, std::chrono::seconds(0), nLoop);
	}
	HSs AGroup::fadeInOut(const HAb& hAb, const Duration fadeIn, const Duration fadeOut, const int nLoop) {
		if(getIdleChannels() > 0) {
			for(auto& s : _source) {
				auto& ch = *s;
				auto st = ch.getState();
				if(st != AState::Playing &&
					st != AState::Paused)
				{
					++_nActive;
					ch.setBuffer(hAb, nLoop);
					ch.setFadeIn(fadeIn);
					if(fadeOut > std::chrono::seconds(0))
						ch.setFadeOut(fadeOut);
					if(!_bPaused)
						ch.play();
					return s;
				}
			}
		}
		return HSs();
	}
	int AGroup::getChannels() const {
		return static_cast<int>(_source.size());
	}
	int AGroup::getIdleChannels() const {
		return getChannels() - _nActive;
	}
	int AGroup::getPlayingChannels() const {
		return _nActive;
	}
	void AGroup::invalidate() {
		_source.clear();
	}
	const char* AGroup::getResourceName() const noexcept {
		return ::rev::resource_name::AGroup;
	}

	namespace {
		template <class T, class Make>
		void MakeAb(const URI& uri, Make&& mk) {
			auto lh = mgr_rw.fromURI(uri, Access::Read);
			mk(lh);
		}
	}
	// ------------------ SoundMgr ------------------
	HAb SoundMgr::loadWaveBatch(const std::string& name) {
		return _buffMgr.loadResourceApp<AWaveBatch>(
			URI(name),
			[](auto& uri, auto&& mk){ MakeAb<AWaveBatch>(uri, mk); }
		).first;
	}
	HAb SoundMgr::loadOggBatch(const std::string& name) {
		return _buffMgr.loadResourceApp<AOggBatch>(
			URI(name),
			[](auto& uri, auto&& mk){ MakeAb<AOggBatch>(uri, mk); }
		).first;
	}
	HAb SoundMgr::loadOggStream(const std::string& name) {
		return _buffMgr.loadResourceApp<AOggStream>(
			URI(name),
			[](auto& uri, auto&& mk){ MakeAb<AOggStream>(uri, mk); }
		).first;
	}
	HSg SoundMgr::createSourceGroup(const int n) {
		return _sgMgr.emplace(n);
	}
	HSs SoundMgr::createSource() {
		return _srcMgr.emplace();
	}
	void SoundMgr::update() {
		for(auto s : _srcMgr)
			s->update();
		for(auto sg : _sgMgr)
			sg->update();
	}
	void SoundMgr::pauseAllSound() {
		for(auto s : _srcMgr)
			s->sys_pause();
	}
	void SoundMgr::resumeAllSound() {
		for(auto s : _srcMgr)
			s->sys_resume();
	}
	void SoundMgr::invalidate() {
		for(auto a : _buffMgr)
			a->invalidate();
		for(auto a : _srcMgr)
			a->invalidate();
		for(auto a : _sgMgr)
			a->invalidate();
	}
}
