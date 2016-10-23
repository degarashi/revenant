#include "sound_depSL_error.hpp"

namespace rev {
	// --------------------- SLError ---------------------
	namespace {
		#define DEF_ERROR_PAIR(name) {name, #name},
		const std::pair<SLresult, const char*> c_slerror[] = {
			DEF_ERROR_PAIR(SL_RESULT_SUCCESS)
			DEF_ERROR_PAIR(SL_RESULT_PRECONDITIONS_VIOLATED)
			DEF_ERROR_PAIR(SL_RESULT_PARAMETER_INVALID)
			DEF_ERROR_PAIR(SL_RESULT_MEMORY_FAILURE)
			DEF_ERROR_PAIR(SL_RESULT_RESOURCE_ERROR)
			DEF_ERROR_PAIR(SL_RESULT_RESOURCE_LOST)
			DEF_ERROR_PAIR(SL_RESULT_IO_ERROR)
			DEF_ERROR_PAIR(SL_RESULT_BUFFER_INSUFFICIENT)
			DEF_ERROR_PAIR(SL_RESULT_CONTENT_CORRUPTED)
			DEF_ERROR_PAIR(SL_RESULT_CONTENT_UNSUPPORTED)
			DEF_ERROR_PAIR(SL_RESULT_CONTENT_NOT_FOUND)
			DEF_ERROR_PAIR(SL_RESULT_PERMISSION_DENIED)
			DEF_ERROR_PAIR(SL_RESULT_FEATURE_UNSUPPORTED)
			DEF_ERROR_PAIR(SL_RESULT_INTERNAL_ERROR)
			DEF_ERROR_PAIR(SL_RESULT_UNKNOWN_ERROR)
			DEF_ERROR_PAIR(SL_RESULT_OPERATION_ABORTED)
			DEF_ERROR_PAIR(SL_RESULT_CONTROL_LOST)
		};
	}
	void SLError::reset() const {}
	const char* SLError::getAPIName() {
		return "OpenSL";
	}
	const char* SLError::errorDesc(SLresult result) {
		if(result != SL_RESULT_SUCCESS) {
			for(auto& p : c_slerror) {
				if(p.first == result)
					return p.second;
			}
			return "unknown error";
		}
		return nullptr;
	}
}
