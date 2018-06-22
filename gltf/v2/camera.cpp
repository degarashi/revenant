#include "gltf/v2/camera.hpp"

namespace rev::gltf::v2 {
	Camera::Camera(const JValue& v):
		gltf::Camera(v),
		Resource(v)
	{}
	Resource::Type Camera::getType() const noexcept {
		return Type::Camera;
	}
}
