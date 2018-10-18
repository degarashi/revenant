#pragma once
#include "gltf/v1/resource.hpp"
#include "gltf/rwref.hpp"

namespace rev::gltf::v1 {
	struct IDataQuery;
	struct Buffer :
		Resource
	{
		RWRef		src;
		std::size_t	byteLength;
		// "arraybuffer" or "text"
		MIME		type;

		Buffer(const JValue& v, const IDataQuery& q);
		Type getType() const noexcept override;
	};
}
