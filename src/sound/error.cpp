#include "error.hpp"
#include <vorbis/vorbisfile.h>

namespace rev {
	// --------------------- OVError ---------------------
	const char* OVError::getAPIName() const noexcept {
		return "OggVorbis";
	}
	const std::pair<int, const char*> OVError::ErrorList[] = {
		{OV_HOLE, "Vorbisfile encoutered missing or corrupt data in the bitstream"},
		{OV_EREAD, "A read from media returned an error"},
		{OV_EFAULT, "Internal logic fault; indicates a bug or heap/stack corruption"},
		{OV_EIMPL, "Feature not implemented"},
		{OV_EINVAL, "Either an invalid argument, or incompletely initialized argument passed to a call"},
		{OV_ENOTVORBIS, "Bitstream does not contain any Vorbis data"},
		{OV_EBADHEADER, "Invalid Vorbis bitstream header"},
		{OV_EVERSION, "Vorbis version mismatch"},
		{OV_EBADLINK, "The given link exists in the Vorbis data stream, but is not decipherable due to garbacge or corruption"},
		{OV_ENOSEEK, "The given stream is not seekable"}
	};
	const char* OVError::errorDesc(const int err) const noexcept {
		if(err < 0) {
			for(auto& p : ErrorList) {
				if(p.first == err)
					return p.second;
			}
			return "unknown error";
		}
		return nullptr;
	}
}
