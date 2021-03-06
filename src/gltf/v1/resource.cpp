#include "gltf/v1/resource.hpp"
#include "../value_loader.hpp"

namespace rev::gltf::v1 {
	Resource::Identity_t Resource::Identity;
	using namespace loader;
	Resource::Resource(Identity_t) {}
	Resource::Resource(const JValue& v):
		username(Optional<String>(v, "name"))
	{}
	namespace {
		const Name c_noname("(noname)");
	}
	const Name& Resource::getName() const noexcept {
		if(username)
			return *username;
		return c_noname;
	}
}
