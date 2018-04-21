#pragma once
#include "resource.hpp"
#include "idtag.hpp"

namespace rev::gltf {
	struct Scene :
		Resource,
		IResolvable
	{
		TagNodeV	node;

		Scene(const JValue& v);
		Type getType() const noexcept override;
		void resolve(const ITagQuery& q) override;
	};
}
