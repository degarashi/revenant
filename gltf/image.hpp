#pragma once
#include "resource.hpp"
#include "idtag.hpp"

namespace rev::gltf {
	struct Image :
		Resource,
		IResolvable
	{
		TagRW			src;

		Image(const JValue& v);
		Type getType() const noexcept override;
		void resolve(const ITagQuery& q) override;
	};
}
