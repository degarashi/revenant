#include "gltf/v1/resource.hpp"
#include "../value_loader.hpp"

namespace rev::gltf::v1 {
	using namespace loader;
	Resource::Resource(const JValue& v):
		username(Optional<String>(v, "name"))
	{}
}
