#pragma once
#include "gltf/v2/resource.hpp"
#include "gltf/camera.hpp"

namespace rev::gltf::v2 {
	class Camera :
		public gltf::Camera,
		public Resource
	{
		public:
			Camera(const JValue& v);
			Type getType() const noexcept override;
	};
}
