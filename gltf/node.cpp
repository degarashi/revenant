#include "gltf/node.hpp"
#include "gltf/camera.hpp"
#include "gltf/mesh.hpp"
#include "gltf/material.hpp"
#include "gltf/skin.hpp"
#include "gltf/accessor.hpp"
#include "../camera3d.hpp"
#include "../dc/node.hpp"

namespace rev::gltf {
	dc::JointId Node::s_id = 1;

	Node::Node(const JValue& v):
		Resource(v),
		pose(loader::Pose3(v)),
		child(loader::Optional<loader::Array<TagNode>>(v, "children", {})),
		jointName(loader::Optional<loader::Shared<loader::StdString>>(v, "jointName", nullptr)),
		jointId(s_id++)
	{}
	Resource::Type Node::getType() const noexcept {
		return Type::Node;
	}
	void Node::resolve(const ITagQuery& q) {
		for(auto& c : child)
			c.resolve(q);
	}
	void Node::_visit(Visitor& v) const {
		auto node = std::make_shared<dc::TfNode>(jointId, jointName,
				username ? *username : Name());
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

	// ------------------- CameraNode -------------------
	std::shared_ptr<CameraNode> CameraNode::Load(const JValue& v) {
		if(v.HasMember("camera"))
			return std::make_shared<CameraNode>(v);
		return nullptr;
	}
	CameraNode::CameraNode(const JValue& v):
		Node(v),
		camera(loader::Required<TagCamera>(v, "camera"))
	{}
	void CameraNode::resolve(const ITagQuery& q) {
		camera.resolve(q);
		Node::resolve(q);
	}
	void CameraNode::visit(Visitor& v) const {
		Node::visit(v);
		v.addCamera(camera.data()->makeCamera());
	}
	// ------------------- MeshNodeBase -------------------
	MeshNodeBase::MeshNodeBase(const JValue& v):
		Node(v),
		mesh(loader::Required<loader::Array<TagMesh>>(v, "meshes"))
	{}
	void MeshNodeBase::resolve(const ITagQuery& q) {
		for(auto& m : mesh)
			m.resolve(q);
		Node::resolve(q);
	}
	// ------------------- MeshNode -------------------
	std::shared_ptr<MeshNode> MeshNode::Load(const JValue& v) {
		if(v.HasMember("meshes"))
			return std::make_shared<MeshNode>(v);
		return nullptr;
	}
	MeshNode::MeshNode(const JValue& v):
		MeshNodeBase(v)
	{}
	void MeshNode::resolve(const ITagQuery& q) {
		for(auto& m : mesh)
			m.resolve(q);
		MeshNodeBase::resolve(q);
	}
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

	// ------------------- SkinMeshNode -------------------
	std::shared_ptr<SkinMeshNode> SkinMeshNode::Load(const JValue& v) {
		if(v.HasMember("skin"))
			return std::make_shared<SkinMeshNode>(v);
		return nullptr;
	}
	SkinMeshNode::SkinMeshNode(const JValue& v):
		MeshNodeBase(v),
		skin(loader::Required<TagSkin>(v, "skin")),
		skeleton(loader::Optional<loader::Array<TagNode>>(v, "skeleton", {}))
	{}
	void SkinMeshNode::resolve(const ITagQuery& q) {
		skin.resolve(q);
		for(auto& s : skeleton)
			s.resolve(q);
		MeshNodeBase::resolve(q);
	}
	void SkinMeshNode::visit(Visitor& v) const {
		Node::visit(v);
		const auto bsm = skin->bindShapeMat;
		const auto& bind = skin.data()->getBind();
		for(auto& m : mesh) {
			for(auto& p : m.data()->primitive) {
				auto& m = *p.material.data();
				v.addSkinMesh(
					p.getPrimitive(),
					m.getTech(),
					username ? *username : Name(),
					m.getRT(),
					bind,
					bsm
				);
			}
		}
	}

	namespace loader {
		Node::Node(const JValue& v) {
			Node_SP& sp = *this;
			if(!v.IsObject())
				throw InvalidProperty("not an object");
			if(auto node = CameraNode::Load(v))
				sp = node;
			else if(auto node = SkinMeshNode::Load(v))
				sp = node;
			else if(auto node = MeshNode::Load(v))
				sp = node;
			else
				sp = std::make_shared<::rev::gltf::Node>(v);
		}
	}
}
