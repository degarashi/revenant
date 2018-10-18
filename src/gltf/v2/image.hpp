#pragma once
#include "gltf/v2/resource.hpp"
#include "gltf/v2/dataref.hpp"
#include "gltf/rwref.hpp"

namespace rev::gltf::v2 {
	class Image :
		public Resource
	{
		public:
			RWRef_OP			src;
			MIME_OP				mimeType;
			DRef_BufferView_OP	bufferView;

			Image(const JValue& v, const IDataQuery& q);
			Type getType() const noexcept override;
	};
}
