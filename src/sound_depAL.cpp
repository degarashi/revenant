#include "sound_depAL.hpp"
#include "sound_depAL_error.hpp"

namespace rev {
	namespace {
		const ALenum c_afmt[] = {
			AL_FORMAT_MONO8,
			AL_FORMAT_MONO16,
			AL_FORMAT_STEREO8,
			AL_FORMAT_STEREO16,
			-1
		};
	}
	ALenum AsALFormat(const AFormat& f) noexcept {
		return c_afmt[static_cast<int>(f.format)];
	}
	// --------------------- ABuffer_depAL ---------------------
	ABuffer_depAL::ABuffer_depAL() {
		ALAssert(alGenBuffers, 1, &_id);
	}
	ABuffer_depAL::ABuffer_depAL(ABuffer_depAL&& a): _id(a._id) {
		a._id = 0;
	}
	ABuffer_depAL::~ABuffer_depAL() {
		if(_id != 0)
			ALWarn(alDeleteBuffers, 1, &_id);
	}
	ALuint ABuffer_depAL::getID() const {
		return _id;
	}
	void ABuffer_depAL::writeBuffer(const AFormatF& af, const void* src, size_t len) {
		D_ALAssert(alBufferData, _id, AsALFormat(af), src, len, af.freq);
	}
	// void ABuffer_depAL::atState(ASource_depAL& src, AState state) {
	// 	if(state == AState::Initial)
	// 		ALECA(alSourcei, src.getID(), AL_BUFFER, getID());
	// }
	// キューのプッシュと残数確認、終了チェック

	// --------------------- ASource_depAL ---------------------
	ASource_depAL::ASource_depAL() {
		ALAssert(alGenSources, 1, &_id);
	}
	ASource_depAL::ASource_depAL(ASource_depAL&& a) noexcept:
		_id(a._id)
	{
		a._id = 0;
	}
	ASource_depAL::~ASource_depAL() {
		if(_id != 0) {
			D_ALWarn(alSourceStop, _id);
			D_ALWarn(alSourcei, _id, AL_BUFFER, 0);
			D_ALWarn(alDeleteSources, 1, &_id);
		}
	}
	void ASource_depAL::play() {
		D_ALAssert(alSourcePlay, _id);
	}
	void ASource_depAL::reset() {
		D_ALAssert(alSourceStop, _id);
		D_ALAssert(alSourceRewind, _id);
	}
	void ASource_depAL::pause() {
		D_ALAssert(alSourcePause, _id);
	}
	void ASource_depAL::update(const bool bPlaying) {
		// もし再生中なのにバッファの途切れなどでSTOPPEDになっていたらPLAYINGにセット
		if(bPlaying) {
			ALint state;
			D_ALAssert(alGetSourcei, _id, AL_SOURCE_STATE, &state);
			if(state == AL_STOPPED)
				D_ALAssert(alSourcePlay, _id);
		}
	}
	void ASource_depAL::setGain(const float vol) {
		D_ALAssert(alSourcef, _id, AL_GAIN, vol);
	}
	void ASource_depAL::setPitch(const float pitch) {
		D_ALAssert(alSourcef, _id, AL_PITCH, pitch);
	}
	Duration ASource_depAL::timeTell(Duration /*def*/) {
		float ret;
		D_ALAssert(alGetSourcef, _id, AL_SEC_OFFSET, &ret);
		return std::chrono::milliseconds(static_cast<uint32_t>(ret * 1000.f));
	}
	int64_t ASource_depAL::pcmTell(int64_t /*def*/) {
		float ret;
		D_ALAssert(alGetSourcef, _id, AL_SAMPLE_OFFSET, & ret);
		return static_cast<int64_t>(ret);
	}
	void ASource_depAL::timeSeek(const float t) {
		D_ALAssert(alSourcef, _id, AL_SEC_OFFSET, t);
	}
	void ASource_depAL::pcmSeek(const int64_t t) {
		// floatでは精度が不足するかもしれないが、int32_tだと桁が足りないかもしれない
		D_ALAssert(alSourcef, _id, AL_SAMPLE_OFFSET, static_cast<ALfloat>(t));
	}
	void ASource_depAL::enqueue(const ABuffer_depAL& buff) {
		ALuint buffID = buff.getID();
		D_ALAssert(alSourceQueueBuffers, _id, 1, &buffID);
	}
	int ASource_depAL::getUsedBlock() {
		int nproc;
		D_ALAssert(alGetSourcei, _id, AL_BUFFERS_PROCESSED, &nproc);
		if(nproc > 0) {
			ALuint ids[MAX_AUDIO_BLOCKNUM];
			D_ALAssert(alSourceUnqueueBuffers, _id, nproc, ids);
		}
		return nproc;
	}
	void ASource_depAL::clearBlock() {
		D_ALAssert(alSourcei, _id, AL_BUFFER, 0);
	}
	ALuint ASource_depAL::getID() const { return _id; }

	void ASource_depAL::setRelativeMode(const bool bRel) {
		D_ALAssert(alSourcei, _id, AL_SOURCE_RELATIVE, bRel ? AL_TRUE : AL_FALSE);
	}
	void ASource_depAL::setPosition(const Vec3& pos) {
		D_ALAssert(alSource3f, _id, AL_POSITION, pos.x, pos.y, pos.z);
	}
	void ASource_depAL::setDirection(const Vec3& dir) {
		D_ALAssert(alSource3f, _id, AL_DIRECTION, dir.x, dir.y, dir.z);
	}
	void ASource_depAL::setVelocity(const Vec3& vel) {
		D_ALAssert(alSource3f, _id, AL_VELOCITY, vel.x, vel.y, vel.z);
	}
	void ASource_depAL::setGainRange(const float gmin, const float gmax) {
		D_ALAssert(alSourcef, _id, AL_MIN_GAIN, gmin);
		D_ALAssert(alSourcef, _id, AL_MAX_GAIN, gmax);
	}
	void ASource_depAL::setAngleGain(const float inner, const float outer) {
		D_ALAssert(alSourcef, _id, AL_CONE_INNER_ANGLE, inner);
		D_ALAssert(alSourcef, _id, AL_CONE_OUTER_ANGLE, outer);
	}
	void ASource_depAL::setAngleOuterGain(const float gain) {
		D_ALAssert(alSourcef, _id, AL_CONE_OUTER_GAIN, gain);
	}

	// --------------------- SoundMgr_depAL ---------------------
	SoundMgr_depAL::SoundMgr_depAL(const int rate):
		_rate(rate)
	{
		_device = alcOpenDevice(nullptr);
		if(!_device)
			throw std::runtime_error("");
	}
	SoundMgr_depAL::~SoundMgr_depAL() {
		for(auto* p : _context) {
			if(p)
				ALCAssert(alcDestroyContext, p);
		}
		alcCloseDevice(_device);
	}
	bool SoundMgr_depAL::operator == (const SoundMgr_depAL& m) const noexcept {
		return _rate == m._rate;
	}
	bool SoundMgr_depAL::operator != (const SoundMgr_depAL& m) const noexcept {
		return !(this->operator == (m));
	}
	ALCdevice* SoundMgr_depAL::getDevice() const {
		return _device;
	}
	int SoundMgr_depAL::getRate() const {
		return _rate;
	}
	void SoundMgr_depAL::printVersions(std::ostream& os) {
		os << "version: " << alGetString(AL_VERSION) << std::endl
			<< "vendor: " << alGetString(AL_VENDOR) << std::endl
			<< "renderer: " << alGetString(AL_RENDERER) << std::endl
			<< "extensions: " << alGetString(AL_EXTENSIONS) << std::endl;
	}
	void SoundMgr_depAL::SetPosition(const Vec3& pos) {
		D_ALAssert(alListener3f, AL_POSITION, pos.x, pos.y, pos.z);
	}
	void SoundMgr_depAL::SetVelocity(const Vec3& v) {
		D_ALAssert(alListener3f, AL_VELOCITY, v.x, v.y, v.z);
	}
	void SoundMgr_depAL::SetGain(const float g) {
		D_ALAssert(alListenerf, AL_GAIN, g);
	}
	void SoundMgr_depAL::SetOrientation(const Vec3& dir, const Vec3& up) {
		const float tmp[] = {
			dir.x, dir.y, dir.z,
			up.x, up.y, up.z
		};
		D_ALAssert(alListenerfv, AL_ORIENTATION, tmp);
	}
	void SoundMgr_depAL::SetDopplerFactor(const float f) {
		// 	alGetFloat(AL_DOPPLER_FACTOR);
		D_ALAssert(alDopplerFactor, f);
	}
	void SoundMgr_depAL::SetSpeedOfSound(const float s) {
		// 	alGetFloat(AL_SPEED_OF_SOUND);
		D_ALAssert(alSpeedOfSound, s);
	}
	void SoundMgr_depAL::SetDistModel(const DistModel model) {
		// 	alGetInteger(AL_DISTANCE_MODEL);
		D_ALAssert(alDistanceModel, static_cast<ALenum>(model));
	}

	SDL_atomic_t SoundMgr_depAL::s_atmThCounter = {};
	TLS<int> SoundMgr_depAL::s_thID;

	void SoundMgr_depAL::makeCurrent() {
		UniLock lk(_mutex);
		if(!s_thID.valid())
			s_thID = SDL_AtomicAdd(&s_atmThCounter, 0x01);
		int idx = s_thID.get();
		int sz = _context.size();
		if(sz <= idx)
			_context.resize(idx+1);
		if(!_context[idx]) {
			const ALint attr[] = {ALC_FREQUENCY, _rate, 0};
			_context[idx] = ALCAssert(alcCreateContext, getDevice(), attr);
		}
		D_ALCAssert(alcMakeContextCurrent, _context[idx]);
	}
	void SoundMgr_depAL::resetCurrent() {
		UniLock lk(_mutex);
		D_ALCAssert(alcMakeContextCurrent, nullptr);
	}
	void SoundMgr_depAL::suspend() {
		UniLock lk(_mutex);
		D_ALCAssert(alcSuspendContext, _context[s_thID.get()]);
	}
	void SoundMgr_depAL::process() {
		UniLock lk(_mutex);
		D_ALCAssert(alcProcessContext, _context[s_thID.get()]);
	}
}
