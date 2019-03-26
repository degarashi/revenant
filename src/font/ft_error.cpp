#include "ft_error.hpp"
#include <utility>

namespace rev {
	namespace {
		#undef __FTERRORS_H__
		#define FT_ERRORDEF(e, v, s) {e, s},
		#define FT_ERROR_START_LIST {
		#define FT_ERROR_END_LIST {0,0} };
		const std::pair<int, const char*> c_ftErrors[] =
		#include FT_ERRORS_H
	}
	// ---------------------- FTError ----------------------
	const char* FTError::errorDesc(int result) const {
		if(result != 0) {
			for(auto& e : c_ftErrors) {
				if(e.first == result)
					return e.second;
			}
			return "unknown error";
		}
		return nullptr;
	}
	const char* FTError::getAPIName() const {
		return "FreeType";
	}
}
