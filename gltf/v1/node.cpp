#include "gltf/v1/node.hpp"
#include "gltf/v1/camera.hpp"
#include "gltf/v1/mesh.hpp"
#include "gltf/v1/material.hpp"
#include "gltf/v1/skin.hpp"
#include "gltf/v1/accessor.hpp"
#include "../../camera3d.hpp"
#include "../../dc/node.hpp"

namespace rev::gltf::v1 {
	namespace L = gltf::loader;
	// --------------------- Node::Visitor ---------------------
	void Node::Visitor::addMesh(const HPrim&, const HTech&, const Name&, const RTUParams_SP&, dc::JointId) {}
	void Node::Visitor::addSkinMesh(const HPrim&, const HTech&, const Name&, const RTUParams_SP&, const SkinBindSet_SP&) {}

	// --------------------- Node ---------------------
	Node::Node():
		Resource(Resource::Identity)
	{}
	Node::Node(const JValue& v, const IDataQuery& q):
		gltf::Node<DRef_Node, IDataQuery>(v, q),
		Resource(v)
	{
		if(const auto jn = L::Optional<L::StdString>(v, "jointName"))
			jointName = *jn;
	}
	Resource::Type Node::getType() const noexcept {
		return Type::Node;
	}
	void Node::visit(Visitor& v) const {
		_Visit(*this, v, [](){});
	}
	void Node::moveTo(void* dst) {
		new(dst) Node(std::move(*this));
	}

	// ------------------- CameraNode -------------------
	std::shared_ptr<CameraNode> CameraNode::Load(const JValue& v, const IDataQuery& q) {
		if(v.HasMember("camera"))
			return std::make_shared<CameraNode>(v, q);
		return nullptr;
	}
	CameraNode::CameraNode(const JValue& v, const IDataQuery& q):
		Node(v, q),
		camera(L::Required<DRef_Camera>(v, "camera", q))
	{}
	void CameraNode::visit(Visitor& v) const {
		_Visit(*this, v, [this, &v](){
			v.addCamera(camera.data()->makeCamera());
		});
	}
	void CameraNode::moveTo(void* dst) {
		new(dst) CameraNode(std::move(*this));
	}
	// ------------------- MeshNodeBase -------------------
	MeshNodeBase::MeshNodeBase(const JValue& v, const IDataQuery& q):
		Node(v, q),
		mesh(L::Required<L::Array<DRef_Mesh>>(v, "meshes", q))
	{}
	// ------------------- MeshNode -------------------
	std::shared_ptr<MeshNode> MeshNode::Load(const JValue& v, const IDataQuery& q) {
		if(v.HasMember("meshes"))
			return std::make_shared<MeshNode>(v, q);
		return nullptr;
	}
	MeshNode::MeshNode(const JValue& v, const IDataQuery& q):
		MeshNodeBase(v, q)
	{}
	void MeshNode::visit(Visitor& v) const {
		_Visit(*this, v, [this, &v](){
			for(auto& m : mesh) {
				const Name name = m->username ? *m->username : Name();
				for(auto& p : m.data()->primitive) {
					auto& mtl = *p.material.data();
					v.addMesh(
						p.getPrimitive(),
						mtl.getTech(),
						name,
						mtl.getRT(),
						jointId
					);
				}
			}
		});
	}
	void MeshNode::moveTo(void* dst) {
		new(dst) MeshNode(std::move(*this));
	}

	// ------------------- SkinMeshNode -------------------
	std::shared_ptr<SkinMeshNode> SkinMeshNode::Load(const JValue& v, const IDataQuery& q) {
		if(v.HasMember("skin"))
			return std::make_shared<SkinMeshNode>(v, q);
		return nullptr;
	}
	SkinMeshNode::SkinMeshNode(const JValue& v, const IDataQuery& q):
		MeshNodeBase(v, q),
		skin(L::Required<DRef_Skin>(v, "skin", q)),
		skeleton(L::OptionalDefault<L::Array<DRef_Node>>(v, "skeleton", {}, q))
	{}
	void SkinMeshNode::visit(Visitor& v) const {
		_Visit(*this, v, [this, &v](){
			const auto& bind = skin.data()->getBind();
			for(auto& m : mesh) {
				for(auto& p : m.data()->primitive) {
					auto& m = *p.material.data();
					v.addSkinMesh(
						p.getPrimitive(),
						m.getTech(),
						username ? *username : Name(),
						m.getRT(),
						bind
					);
				}
			}
		});
	}
	void SkinMeshNode::moveTo(void* dst) {
		new(dst) SkinMeshNode(std::move(*this));
	}
}
