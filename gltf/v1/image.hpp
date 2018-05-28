#pragma once
#include "gltf/v1/resource.hpp"
#include "gltf/v1/rwref.hpp"

namespace rev::gltf::v1 {
	struct Image :
		Resource
	{
		RWRef		src;

		Image(const JValue& v, const IDataQuery& q);
		Type getType() const noexcept override;
	};
}
