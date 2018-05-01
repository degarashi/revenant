#include "dc/model.hpp"
#include "dc/mesh.hpp"
#include "dc/node.hpp"

namespace rev::dc {
	Model::Model(const MeshV& mesh, const HTf& tf):
		_mesh(mesh),
		_tf(tf)
	{}
	void Model::draw(IEffect& e) const {
		const rev::dc::NodeParam_cached npc(*_tf);
		for(auto& m : _mesh) {
			m->draw(e, npc);
		}
	}
	const HTf& Model::getNode() const noexcept {
		return _tf;
	}
}
