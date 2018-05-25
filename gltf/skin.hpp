#pragma once
#include "resource.hpp"
#include "idtag.hpp"
#include "frea/matrix.hpp"

namespace rev::gltf {
	struct SkinBindSet;
	using SkinBindSet_SP = std::shared_ptr<SkinBindSet>;
	struct Skin :
		Resource,
		IResolvable
	{
		frea::Mat4		bindShapeMat;
		TagAccessor		invBindMat;
		SName_V			jointName;

		mutable SkinBindSet_SP	bind_cached;

		Skin(const JValue& v);
		Type getType() const noexcept override;
		void resolve(const ITagQuery& q) override;

		const SkinBindSet_SP& getBind() const;
	};
}
