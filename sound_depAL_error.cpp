#include "sound_depAL_error.hpp"

namespace rev {
	// --------------------- ALError ---------------------
	const std::pair<ALenum, const char*> ALError::ErrorList[] = {
		{AL_NO_ERROR, "No error"},
		{AL_INVALID_NAME, "Invalid name paramater passed to AL call"},
		{AL_INVALID_ENUM, "Invalid enum parameter passed to AL call"},
		{AL_INVALID_VALUE, "Invalid value parameter passed to AL call"},
		{AL_INVALID_OPERATION, "Illegal AL call"},
		{AL_OUT_OF_MEMORY, "Not enough memory"}
	};
	void ALError::reset() const {
		if(alcGetCurrentContext() == nullptr)
			return;
		while(alGetError() != AL_NO_ERROR);
	}
	const char* ALError::getAPIName() const {
		return "OpenAL";
	}
	const char* ALError::errorDesc() const {
		ALenum num = alGetError();
		if(num != AL_NO_ERROR) {
			for(auto& p : ErrorList) {
				if(p.first == num)
					return p.second;
			}
			return "unknown error";
		}
		return nullptr;
	}
	// --------------------- ALCError ---------------------
	ALCError::ALCError(ALCdevice* dev): _dev(dev) {}
	const std::pair<ALCenum, const char*> ALCError::ErrorList[] = {
		{ALC_NO_ERROR, "No error"},
		{ALC_INVALID_DEVICE, "Invalid device handle"},
		{ALC_INVALID_CONTEXT, "Invalid context handle"},
		{ALC_INVALID_ENUM, "Invalid enum parameter passed to an ALC call"},
		{ALC_INVALID_VALUE, "Invalid value parameter passed to an ALC call"},
		{ALC_OUT_OF_MEMORY, "Out of memory"}
	};
	void ALCError::reset() const {
		while(alcGetError(_dev) != ALC_NO_ERROR);
	}
	const char* ALCError::getAPIName() const {
		return "OpenAL_C";
	}
	const char* ALCError::errorDesc() const {
		ALCenum num = alcGetError(_dev);
		if(num != ALC_NO_ERROR) {
			for(auto& p : ErrorList) {
				if(p.first == num)
					return p.second;
			}
			return "unknown error";
		}
		return nullptr;
	}
}
