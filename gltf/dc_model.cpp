#include "dc_model.hpp"
#include "dc_mesh.hpp"
#include "gltf/node_cached.hpp"
#include "../dc/node.hpp"
#include "../dc/node_cached.hpp"
#include "../systeminfo.hpp"
#include "../u_matrix3d.hpp"
#include "../glx_if.hpp"
#include "../fbrect.hpp"
#include "../dc/node_cached.hpp"

namespace rev::gltf {
	GLTFModel::GLTFModel(const MeshV& mesh, const HTf& tf) {
		_mesh = mesh;
		_tf = tf;
	}
	void GLTFModel::draw(IEffect& e) const {
		dc::NodeParam_cached np(*_tf);
		const auto cam = dynamic_cast<const U_Matrix3D&>(e).getCamera();
		const auto vp = e.getViewport().resolve([](){ return mgr_info.getScreenSize(); });
		NodeParam_USemCached npc(cam, vp, np);

		for(auto& m : _mesh) {
			auto& gm = static_cast<const GLTFMesh&>(*m);
			np.setNodeJointId(gm._jointId);
			m->draw(e, npc);
		}
	}
	HTf GLTFModel::getNode() const {
		return _tf;
	}
}
