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
		auto* ret = static_cast<dc::NodeParam*>(static_cast<NodeParam_USem*>(new NodeParam_USemCached(cam, vp, np)));
		auto npc = std::unique_ptr<dc::NodeParam>(ret);

		for(auto& m : _mesh) {
			m->draw(e, *npc);
		}
	}
}
