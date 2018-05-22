#include "dc_model.hpp"
#include "dc_mesh.hpp"
#include "gltf/qm_usemcached.hpp"
#include "../dc/node.hpp"
#include "../systeminfo.hpp"
#include "../u_matrix3d.hpp"
#include "../glx_if.hpp"
#include "../fbrect.hpp"
#include "gltf/visitor_model.hpp"
#include "gltf/scene.hpp"
#include "frea/vector.hpp"

namespace rev::gltf {
	GLTFModel::GLTFModel(const MeshV& mesh, const MeshV& skinmesh, const HTf& tf):
		_mesh(mesh),
		_skinmesh(skinmesh),
		_tf(tf),
		_qm(*_tf)
	{}
	void GLTFModel::draw(IEffect& e) const {
		const auto cam = dynamic_cast<const U_Matrix3D&>(e).getCamera();
		const auto vp = e.getViewport().resolve([](){ return mgr_info.getScreenSize(); });
		QueryMatrix_USemCached qmc(cam, vp, _qm.prepareInterface());

		for(auto& m : _mesh) {
			m->draw(e, qmc);
		}
		for(auto& m : _skinmesh) {
			m->draw(e, qmc);
		}
	}
	HTf GLTFModel::getNode() const {
		return _tf;
	}

	HMdl GLTFModel::FromScene(const Scene& s) {
		gltf::Visitor_Model visitor;
		{
			auto node = std::make_shared<dc::TfNode>(0, SName(), "RootNode");
			node->refPose().identity();
			node->refPose().setScaling(frea::Vec3{1,1,-1});
			visitor.addNode(node);
		}
		for(auto& n : s.node) {
			n->visit(visitor);
		}
		return visitor.result();
	}
}
