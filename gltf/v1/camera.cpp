#include "gltf/v1/camera.hpp"

namespace rev::gltf::v1 {
	Camera::Camera(const JValue& v):
		gltf::Camera(v),
		Resource(v)
	{}
	Resource::Type Camera::getType() const noexcept {
		return Type::Camera;
	}
}
