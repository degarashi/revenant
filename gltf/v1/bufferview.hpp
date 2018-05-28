#pragma once
#include "resource.hpp"
#include "dataref.hpp"
#include "../../gl_format.hpp"
#include "../../gl_types.hpp"

namespace rev::gltf::v1 {
	struct BufferView :
		Resource
	{
		DRef_Buffer		src;
		std::size_t		byteOffset,
						byteLength;
		// VertexBuffer or IndexBuffer
		using BufferType_OP = spi::Optional<BufferType>;
		BufferType_OP	target;

		BufferView(const JValue& v, const IDataQuery& q);
		Type getType() const noexcept override;

		std::pair<uintptr_t, std::size_t> getBuffer() const;
	};
}
