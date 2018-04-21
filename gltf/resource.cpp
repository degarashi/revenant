#include "gltf/resource.hpp"
#include "gltf/value_loader.hpp"

namespace rev {
	namespace gltf {
		using namespace loader;
		Resource::Resource(const JValue& v):
			name(g_dictEntName),
			username(Optional<String>(v, "name"))
		{}
	}
}
