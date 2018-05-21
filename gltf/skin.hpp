#pragma once
#include "resource.hpp"
#include "idtag.hpp"
#include "frea/matrix.hpp"
#include "../dc/common.hpp"

namespace rev::gltf {
	struct Skin :
		Resource,
		IResolvable
	{
		frea::Mat4		bindShapeMat;
		TagAccessor		invBindMat;
		SName_V			jointName;

		mutable dc::SkinBindSet_SP	bind_cached;

		Skin(const JValue& v);
		Type getType() const noexcept override;
		void resolve(const ITagQuery& q) override;

		const dc::SkinBindSet_SP& getBind() const;
	};
}
