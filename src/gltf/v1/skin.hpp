#pragma once
#include "gltf/v1/dataref.hpp"
#include "frea/src/matrix.hpp"

namespace rev::gltf::v1 {
	struct SkinBindSet;
	using SkinBindSet_SP = std::shared_ptr<SkinBindSet>;
	struct Skin :
		Resource
	{
		frea::Mat4		bindShapeMat;
		DRef_Accessor	invBindMat;
		SName_V			jointName;

		mutable SkinBindSet_SP	bind_cached;

		Skin(const JValue& v, const IDataQuery& q);
		Type getType() const noexcept override;

		const SkinBindSet_SP& getBind() const;
	};
}
