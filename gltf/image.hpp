#pragma once
#include "gltf/resource.hpp"
#include "gltf/rwref.hpp"

namespace rev::gltf {
	struct Image :
		Resource
	{
		RWRef		src;

		Image(const JValue& v, const IDataQuery& q);
		Type getType() const noexcept override;
	};
}
