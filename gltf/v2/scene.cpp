#include "gltf/v2/scene.hpp"
#include "../value_loader.hpp"

namespace rev::gltf::v2 {
	namespace L = gltf::loader;
	Scene::Scene(const JValue& v, const IDataQuery& q):
		Resource(v),
		node(L::OptionalDefault<L::Array<DRef_Node>>(v, "nodes", {}, q))
	{}
	Resource::Type Scene::getType() const noexcept {
		return Type::Scene;
	}
}
