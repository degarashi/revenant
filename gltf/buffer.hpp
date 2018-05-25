#pragma once
#include "gltf/resource.hpp"
#include "gltf/rwref.hpp"
#include "../handle/opengl.hpp"

namespace rev::gltf {
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
