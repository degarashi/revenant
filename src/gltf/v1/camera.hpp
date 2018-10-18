#pragma once
#include "gltf/v1/resource.hpp"
#include "gltf/camera.hpp"

namespace rev::gltf::v1 {
	struct Camera :
		gltf::Camera,
		Resource
	{
		Camera(const JValue& v);
		Type getType() const noexcept override;
	};
}
