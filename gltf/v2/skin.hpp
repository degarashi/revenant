#pragma once
#include "gltf/v2/resource.hpp"
#include "gltf/v2/dataref.hpp"

namespace rev::gltf::v2 {
	struct SkinBindSet;
	using SkinBindSet_SP = std::shared_ptr<SkinBindSet>;
	class Skin :
		public Resource
	{
		public:
			using DRef_NodeV = std::vector<DRef_Node>;

			DRef_Accessor_OP	invBindMat;
			DRef_Node_OP		skeleton;
			DRef_NodeV			joints;

			mutable SkinBindSet_SP	bind_cached;

			Skin(const JValue& v, const IDataQuery& q);
			Type getType() const noexcept override;

			const SkinBindSet_SP& getBind() const;
	};
}
