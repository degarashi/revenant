#include "gltf/scene.hpp"
#include "gltf/value_loader.hpp"

namespace rev::gltf {
	using namespace loader;
	Scene::Scene(const JValue& v, const IDataQuery& q):
		Resource(v),
		node(OptionalDefault<Array<DRef_Node>>(v, "nodes", {}, q))
	{}
	Resource::Type Scene::getType() const noexcept {
		return Type::Scene;
	}
}
