#include "gltf/v1/node.hpp"
#include "gltf/v1/camera.hpp"
#include "gltf/v1/mesh.hpp"
#include "gltf/v1/material.hpp"
#include "gltf/v1/skin.hpp"
#include "gltf/v1/accessor.hpp"
#include "../../camera3d.hpp"
#include "../../dc/node.hpp"

namespace rev::gltf::v1 {
	dc::JointId Node::s_id = 1;

	Node::Node(const JValue& v, const IDataQuery& q):
		Resource(v),
		pose(loader::Pose3(v)),
		child(loader::OptionalDefault<loader::Array<DRef_Node>>(v, "children", {}, q)),
		jointId(s_id++)
	{
		if(const auto jn = loader::Optional<loader::StdString>(v, "jointName"))
			jointName = *jn;
	}
	Resource::Type Node::getType() const noexcept {
		return Type::Node;
	}
	void Node::_visit(Visitor& v) const {
		auto node = std::make_shared<dc::TfNode>(jointId, jointName, username ? *username : Name());
		node->setPose(pose);
		v.addNode(node);
	}
	void Node::visit(Visitor& v) const {
		_visit(v);
		for(auto& c : child) {
			c.data()->visit(v);
		}
		v.upNode();
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
		camera(loader::Required<DRef_Camera>(v, "camera", q))
	{}
	void CameraNode::visit(Visitor& v) const {
		Node::visit(v);
		v.addCamera(camera.data()->makeCamera());
	}
	void CameraNode::moveTo(void* dst) {
		new(dst) CameraNode(std::move(*this));
	}
	// ------------------- MeshNodeBase -------------------
	MeshNodeBase::MeshNodeBase(const JValue& v, const IDataQuery& q):
		Node(v, q),
		mesh(loader::Required<loader::Array<DRef_Mesh>>(v, "meshes", q))
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
		Node::visit(v);
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
		skin(loader::Required<DRef_Skin>(v, "skin", q)),
		skeleton(loader::OptionalDefault<loader::Array<DRef_Node>>(v, "skeleton", {}, q))
	{}
	void SkinMeshNode::visit(Visitor& v) const {
		Node::visit(v);
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
	}
	void SkinMeshNode::moveTo(void* dst) {
		new(dst) SkinMeshNode(std::move(*this));
	}

	namespace loader {
		Node::Node(const JValue& v, const IDataQuery& q) {
			Node_SP& sp = *this;
			if(!v.IsObject())
				throw InvalidProperty("not an object");
			if(auto node = CameraNode::Load(v, q))
				sp = node;
			else if(auto node = SkinMeshNode::Load(v, q))
				sp = node;
			else if(auto node = MeshNode::Load(v, q))
				sp = node;
			else
				sp = std::make_shared<::rev::gltf::v1::Node>(v, q);
		}
	}
}
