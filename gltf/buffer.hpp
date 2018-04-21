#pragma once
#include "resource.hpp"
#include "idtag.hpp"
#include "../handle/opengl.hpp"

namespace rev::gltf {
	struct Buffer :
		Resource,
		IResolvable
	{
		TagRW		src;
		std::size_t	byteLength;
		// "arraybuffer" or "text"
		MIME		type;

		Buffer(const JValue& v);
		Type getType() const noexcept override;
		void resolve(const ITagQuery& q) override;

		mutable HVb		vb_cached;
		const HVb& getAsVb() const;
	};
}
