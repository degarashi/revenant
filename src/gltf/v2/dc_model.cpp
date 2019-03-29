#include "gltf/v2/dc_model.hpp"
#include "gltf/v2/visitor_model.hpp"
#include "gltf/v2/scene.hpp"
#include "gltf/v2/dc_mesh.hpp"
#include "../../dc/node.hpp"
#include "../../effect/uniform/matrix3d.hpp"
#include "../../effect/if.hpp"
#include "../../fbrect.hpp"
#include "../../systeminfo.hpp"

namespace rev::gltf::v2 {
	GLTFModel::GLTFModel(const MeshV& mesh, const MeshV& skinmesh, const HTf& tf):
		_mesh(mesh),
		_skinmesh(skinmesh),
		_tf(tf),
		_qm(*_tf)
	{}
	void GLTFModel::draw(IEffect& e) const {
		const dc::IQueryMatrix& qm = _qm.prepareInterface();
		dc::BBox*	bb = nullptr;
		using L = std::numeric_limits<float>;
		dc::BBox	bb_tmp{
					.min = frea::Vec3(L::max()),
					.max = frea::Vec3(L::min())
				};
		if(!_bsphere)
			bb = &bb_tmp;
		for(auto& m : _mesh) {
			m->draw(e, qm, bb);
		}
		for(auto& m : _skinmesh) {
			m->draw(e, qm, bb);
		}
		if(!_bsphere)
			_bsphere = bb->makeSphere();
	}
	HTf GLTFModel::getNode() const {
		return _tf;
	}
	HMdl GLTFModel::FromScene(const Scene& s) {
		gltf::v2::Visitor_Model visitor;
		{
			Node node;
			node.jointId = 0;
			node.name = "RootNode";
			node.pose.identity();
			node.pose.setScaling(frea::Vec3{1,1,-1});
			visitor.addNode(node);
		}
		for(auto& n : s.node) {
			n->visit(visitor);
		}
		return visitor.result();
	}
	dc::BSphere_Op GLTFModel::getBSphere() const {
		return _bsphere;
	}
}
