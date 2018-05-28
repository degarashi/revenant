#pragma once
#include "gltf/v1/resource.hpp"
#include "gltf/v1/rwref.hpp"
#include "../../handle/opengl.hpp"

namespace rev::gltf::v1 {
	struct Buffer :
		Resource
	{
		RWRef		src;
		std::size_t	byteLength;
		// "arraybuffer" or "text"
		MIME		type;

		Buffer(const JValue& v, const IDataQuery& q);
		Type getType() const noexcept override;

		mutable HVb		vb_cached;
		const HVb& getAsVb() const;
	};
}
