#include "dc_model.hpp"
#include "dc_mesh.hpp"
#include "gltf/v1/qm_usemcached.hpp"
#include "gltf/v1/scene.hpp"
#include "gltf/v1/visitor_model.hpp"
#include "../../dc/node.hpp"
#include "../../effect/if.hpp"
#include "../../effect/uniform/matrix3d.hpp"
#include "../../fbrect.hpp"
#include "../../systeminfo.hpp"
#include "frea/src/vector.hpp"

namespace rev::gltf::v1 {
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
		bool clear = false;

		for(auto& m : _mesh) {
			clear |= m->draw(e, qmc);
		}
		for(auto& m : _skinmesh) {
			clear |= m->draw(e, qmc);
		}
		if(clear)
			_qm.clearCache();
	}
	HTf GLTFModel::getNode() const {
		return _tf;
	}

	HMdl GLTFModel::FromScene(const Scene& s) {
		gltf::v1::Visitor_Model visitor;
		{
			Node node;
			node.jointId = 0;
			node.username = "RootNode";
			node.pose.identity();
			node.pose.setScaling(frea::Vec3{1,1,-1});
			visitor.addNode(node);
		}
		for(auto& n : s.node) {
			n->visit(visitor);
		}
		return visitor.result();
	}
}
