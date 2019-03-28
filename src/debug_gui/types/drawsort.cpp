#include "../../effect/drawsort.hpp"

namespace rev {
	const char* DSort_Z_Asc::getDebugName() const noexcept {
		return "Z_Ascend";
	}
	const char* DSort_Z_Desc::getDebugName() const noexcept {
		return "Z_Descend";
	}
	const char* DSort_Priority_Asc::getDebugName() const noexcept {
		return "Priority_Ascend";
	}
	const char* DSort_Priority_Desc::getDebugName() const noexcept {
		return "Priority_Descend";
	}
	const char* DSort_TechPass::getDebugName() const noexcept {
		return "TechPass";
	}
	const char* DSort_Texture::getDebugName() const noexcept {
		return "Texture";
	}
	const char* DSort_Primitive::getDebugName() const noexcept {
		return "Primitive";
	}
}
