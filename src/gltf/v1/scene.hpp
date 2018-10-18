#pragma once
#include "gltf/v1/resource.hpp"
#include "gltf/v1/dataref.hpp"

namespace rev::gltf::v1 {
	struct Scene :
		Resource
	{
		using DRef_NodeV = std::vector<DRef_Node>;
		DRef_NodeV	node;

		Scene(const JValue& v, const IDataQuery& q);
		Type getType() const noexcept override;
	};
}
