#include "gltf/v2/resource.hpp"
#include "../value_loader.hpp"

namespace rev::gltf::v2 {
	namespace L = gltf::loader;
	Resource::Resource(std::nullptr_t) {}
	Resource::Resource(const JValue& v):
		name(L::Optional<L::String>(v, "name"))
	{}
	bool Resource::CanLoad(const JValue& v) noexcept {
		return v.IsObject();
	}
	namespace {
		const Name c_noname("(noname)");
	}
	const Name& Resource::getName() const noexcept {
		if(name)
			return *name;
		return c_noname;
	}
}
