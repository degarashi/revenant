#pragma once
#include "dataref.hpp"
#include "frea/matrix.hpp"

namespace rev::gltf {
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
