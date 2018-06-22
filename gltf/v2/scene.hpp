#pragma once
#include "gltf/v2/resource.hpp"
#include "gltf/v2/dataref.hpp"

namespace rev::gltf::v2 {
	class Scene :
		public Resource
	{
		public:
			using DRef_NodeV = std::vector<DRef_Node>;
			DRef_NodeV	node;

			Scene(const JValue& v, const IDataQuery& q);
			Type getType() const noexcept override;
	};
}
