#include "gltf/resource.hpp"
#include "gltf/value_loader.hpp"

namespace rev::gltf {
	using namespace loader;
	Resource::Resource(const JValue& v):
		username(Optional<String>(v, "name"))
	{}
}
