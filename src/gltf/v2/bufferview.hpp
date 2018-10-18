#pragma once
#include "gltf/v2/dataref.hpp"
#include "../bufferview.hpp"

namespace rev::gltf::v2 {
	class BufferView :
		public gltf::BufferView,
		public Resource
	{
		public:
			DRef_Buffer		buffer;
			Size_OP			byteStride;

			BufferView(const JValue& v, const IDataQuery& q);
			Type getType() const noexcept override;
			DataP getBuffer() const override;
	};
}
