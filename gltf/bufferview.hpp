#pragma once
#include "resource.hpp"
#include "idtag.hpp"
#include "../gl_format.hpp"
#include "../gl_types.hpp"

namespace rev::gltf {
	struct BufferView :
		Resource,
		IResolvable
	{
		TagBuffer		src;
		std::size_t		byteOffset,
						byteLength;
		// VertexBuffer or IndexBuffer
		using BufferType_OP = spi::Optional<BufferType>;
		BufferType_OP	target;

		BufferView(const JValue& v);
		Type getType() const noexcept override;
		void resolve(const ITagQuery& q) override;

		std::pair<uintptr_t, std::size_t> getBuffer() const;
	};
}
