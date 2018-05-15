#include "dc/model.hpp"
#include "dc/mesh.hpp"
#include "dc/node.hpp"
#include "dc/node_cached.hpp"

namespace rev::dc {
	NodeParam_UP Model::DefaultCache(const IEffect&, const NodeParam& np) {
		return std::make_unique<NodeParam_cached>(np);
	}

	Model::Model(const MeshV& mesh, const HTf& tf, const MakeCacheF mc):
		_mesh(mesh),
		_tf(tf),
		_mc(mc)
	{}
	void Model::draw(IEffect& e) const {
		const auto npc = _mc(e, *_tf);
		for(auto& m : _mesh) {
			m->draw(e, *npc);
		}
	}
	const HTf& Model::getNode() const noexcept {
		return _tf;
	}
}
