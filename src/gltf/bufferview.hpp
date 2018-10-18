#pragma once
#include "../handle/opengl.hpp"
#include "../handle/sdl.hpp"
#include "../gl_types.hpp"
#include "json_types.hpp"
#include "data_pair.hpp"

namespace rev::gltf {
	struct BufferView {
		using BufferType_OP = spi::Optional<BufferType>;

		std::size_t		byteOffset,
						byteLength;
		// VertexBuffer or IndexBuffer
		BufferType_OP	target;
		mutable HVb		vb_cached;

		BufferView(const JValue& v);
		const HVb& getAsVb() const;
		virtual DataP getBuffer() const = 0;
		HRW getAsRW() const;
	};
}
