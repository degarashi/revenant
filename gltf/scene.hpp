#pragma once
#include "gltf/resource.hpp"
#include "gltf/dataref.hpp"

namespace rev::gltf {
	struct Scene :
		Resource
	{
		using DRef_NodeV = std::vector<DRef_Node>;
		DRef_NodeV	node;

		Scene(const JValue& v, const IDataQuery& q);
		Type getType() const noexcept override;
	};
}
