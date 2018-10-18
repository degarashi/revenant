#pragma once
#include <cstdint>

namespace rev::gltf {
	struct DataP {
		uintptr_t	pointer;
		std::size_t	length;

		void* asPointer() noexcept {
			return reinterpret_cast<void*>(pointer);
		}
		const void* asPointer() const noexcept {
			return reinterpret_cast<const void*>(pointer);
		}
	};
	struct DataP_Unit : DataP {
		std::size_t		unitSize;
	};
}
