#include "gltf/v2/node.hpp"
#include "gltf/v2/camera.hpp"
#include "gltf/v2/mesh.hpp"
#include "gltf/v2/material.hpp"
#include "gltf/v2/skin.hpp"
#include "../value_loader.hpp"

namespace rev::gltf::v2 {
	// --------------------- Node::Visitor ---------------------
	void Node::Visitor::addMesh(const HGMesh&) {}
	void Node::Visitor::addSkinMesh(const HGMesh&) {}

	namespace L = gltf::loader;
	// --------------------- Node ---------------------
	Node::Node():
		Resource(nullptr)
	{}
	Node::Node(const JValue& v, const IDataQuery& q):
		gltf::Node<DRef_Node, IDataQuery>(v, q),
		Resource(v)
	{}
	Resource::Type Node::getType() const noexcept {
		return Type::Node;
	}
	void Node::moveTo(void* dst) {
		new(dst) Node(std::move(*this));
	}
	void Node::visit(Visitor& v) const {
		_Visit(*this, v, [](){});
	}
	void Node::visit(VisitorBase& v) const {
		_Visit(*this, v, [](){});
	}

	// --------------------- CameraNode ---------------------
	SPtr<CameraNode> CameraNode::Load(const JValue& v, const IDataQuery& q) {
		if(v.HasMember("camera"))
			return std::make_shared<CameraNode>(v, q);
		return nullptr;
	}
	CameraNode::CameraNode(const JValue& v, const IDataQuery& q):
		Node(v, q),
		camera(L::Required<DRef_Camera>(v, "camera", q))
	{}
	void CameraNode::moveTo(void* dst) {
		new(dst) CameraNode(std::move(*this));
	}
	void CameraNode::visit(Visitor& v) const {
		_Visit(*this, v, [this, &v](){
			v.addCamera(camera.data()->makeCamera());
		});
	}
	void CameraNode::visit(VisitorBase& v) const {
		_Visit(*this, v, [this, &v](){
			v.addCamera(camera.data()->makeCamera());
		});
	}

	// --------------------- MeshNodeBase ---------------------
	MeshNodeBase::MeshNodeBase(const JValue& v, const IDataQuery& q):
		Node(v, q),
		mesh(L::Required<DRef_Mesh>(v, "mesh", q))
	{
		if(const auto w = L::Optional<L::Array<L::Float>>(v, "weights"))
			weight = std::make_shared<WeightV>(*w);
	}

	// --------------------- MeshNode ---------------------
	SPtr<MeshNode> MeshNode::Load(const JValue& v, const IDataQuery& q) {
		if(v.HasMember("mesh"))
			return std::make_shared<MeshNode>(v, q);
		return nullptr;
	}
	MeshNode::MeshNode(const JValue& v, const IDataQuery& q):
		MeshNodeBase(v, q)
	{}
	void MeshNode::moveTo(void* dst) {
		new(dst) MeshNode(std::move(*this));
	}
	void MeshNode::visit(Visitor& v) const {
		WeightV_S w;
		if(this->weight)
			w = this->weight;
		else if(mesh->weight)
			w = mesh->weight;
		auto& self = const_cast<MeshNode&>(*this);
		self.userData = w;

		_Visit(*this, v, [this, &v, &w](){
			const Name& name = mesh->getName();
			for(auto& p : mesh->primitives) {
				v.addMesh(MakeMesh(p, w, name, jointId));
			}
		});
	}

	// --------------------- SkinMeshNode ---------------------
	SPtr<SkinMeshNode> SkinMeshNode::Load(const JValue& v, const IDataQuery& q) {
		if(v.HasMember("skin"))
			return std::make_shared<SkinMeshNode>(v, q);
		return nullptr;
	}
	SkinMeshNode::SkinMeshNode(const JValue& v, const IDataQuery& q):
		MeshNodeBase(v, q),
		skin(L::Required<DRef_Skin>(v, "skin", q))
	{}
	void SkinMeshNode::moveTo(void* dst) {
		new(dst) SkinMeshNode(std::move(*this));
	}
	void SkinMeshNode::visit(Visitor& v) const {
		// ---- copy-pasted ----
		WeightV_S w;
		if(this->weight)
			w = this->weight;
		else if(mesh->weight)
			w = mesh->weight;
		auto& self = const_cast<SkinMeshNode&>(*this);
		self.userData = w;
		// -----------------

		_Visit(*this, v, [this, &v, &w](){
			const Name& name = mesh->getName();
			const auto& bind = skin->getBind();
			for(auto& p : mesh->primitives) {
				v.addSkinMesh(MakeMesh(p, w, name, bind));
			}
		});
	}
}
