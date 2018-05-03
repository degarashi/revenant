#include "gltf/visitor_model.hpp"
#include "gltf/dc_mesh.hpp"
#include "../dc/node.hpp"
#include "../dc/model.hpp"
#include "gltf/node_cached.hpp"
#include "glx_if.hpp"
#include "sys_uniform3d.hpp"
#include "fbrect.hpp"
#include "systeminfo.hpp"

namespace rev::gltf {
	Visitor_Model::Visitor_Model():
		_tfRoot(std::make_shared<dc::TfRoot>())
	{}
	void Visitor_Model::upNode() {
		_stack.pop_back();
	}
	void Visitor_Model::addNode(const HTfNode& node) {
		if(_stack.empty()) {
			_tfRoot->refNode().emplace_back(node);
		} else {
			_stack.back()->addChild(node);
		}
		_stack.emplace_back(node.get());
	}
	void Visitor_Model::addMesh(const HPrim& p, const HTech& t, const Name& userName, const RTUParams_SP& rt, const dc::JointId id) {
		_mesh.emplace_back(new GLTFMesh(p, t, userName, rt, id));
	}
	void Visitor_Model::addSkinMesh(const HPrim& p, const HTech& t, const Name& userName, const RTUParams_SP& rt, const dc::SkinBindV_SP& bind, const frea::Mat4& bsm) {
		_mesh.emplace_back(new GLTFMesh(p, t, userName, rt, bind, bsm));
	}
	void Visitor_Model::addCamera(const HCam3&) {}
	namespace {
		dc::NodeParam_UP MakeCache(const IEffect& e, const dc::NodeParam& np) {
			const auto cam = dynamic_cast<const SystemUniform3D&>(e).getCamera();
			const auto vp = e.getViewport().resolve([](){ return mgr_info.getScreenSize(); });
			auto* ret = static_cast<dc::NodeParam*>(static_cast<NodeParam_USem*>(new NodeParam_USemCached(cam, vp, np)));
			return dc::NodeParam_UP(ret);
		}
	}
	HMdl Visitor_Model::result() const {
		auto* self = const_cast<Visitor_Model*>(this);
		// メッシュをTechでソート
		std::sort(self->_mesh.begin(), self->_mesh.end(), [](const auto& m0, const auto& m1){
			return m0->getTech().get() < m1->getTech().get();
		});
		return std::make_shared<dc::Model>(_mesh, _tfRoot, &MakeCache);
	}
}
