#include "gltf/scene.hpp"
#include "gltf/value_loader.hpp"

namespace rev::gltf {
	using namespace loader;
	Scene::Scene(const JValue& v):
		Resource(v),
		node(Optional<Array<TagNode>>(v, "nodes", {}))
	{}
	Resource::Type Scene::getType() const noexcept {
		return Type::Scene;
	}
	void Scene::resolve(const ITagQuery& q) {
		for(auto& n : node)
			n.resolve(q);
	}
}
