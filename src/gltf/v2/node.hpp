#pragma once
#include "gltf/v2/resource.hpp"
#include "gltf/v2/dataref.hpp"
#include "gltf/v2/dc_mesh_if.hpp"
#include "gltf/node.hpp"
#include "../../handle/camera.hpp"

namespace rev {
	struct PBRMetallicRoughness;
	using PBR_SP = std::shared_ptr<PBRMetallicRoughness>;
}
namespace rev::gltf::v2 {
	struct SkinBindSet;
	using SkinBindSet_SP = std::shared_ptr<SkinBindSet>;
	class Node :
		public gltf::Node<DRef_Node, IDataQuery>,
		public Resource
	{
		public:
			using base_t = gltf::Node<DRef_Node, IDataQuery>;
			using VisitorBase = NodeVisitor<Node>;
			struct Visitor : VisitorBase {
				virtual void addMesh(const HGMesh& m);
				virtual void addSkinMesh(const HGMesh& m);
			};

			Node();
			Node(const JValue& v, const IDataQuery& q);
			Type getType() const noexcept override;
			virtual void moveTo(void* dst);
			virtual void visit(Visitor& v) const;
			virtual void visit(VisitorBase& v) const;
	};

	template <class T>
	using SPtr = std::shared_ptr<T>;
	struct CameraNode : Node {
		DRef_Camera			camera;

		static SPtr<CameraNode> Load(const JValue& v, const IDataQuery& q);
		CameraNode(const JValue& v, const IDataQuery& q);
		void moveTo(void* dst) override;
		void visit(Visitor& v) const override;
		void visit(VisitorBase& v) const override;
	};
	struct MeshNodeBase : Node {
		DRef_Mesh		mesh;
		WeightV_S		weight;

		MeshNodeBase(const JValue& v, const IDataQuery& q);
	};
	struct MeshNode : MeshNodeBase {
		MeshNode(const JValue& v, const IDataQuery& q);

		static SPtr<MeshNode> Load(const JValue& v, const IDataQuery& q);
		void moveTo(void* dst) override;
		void visit(Visitor& v) const override;
	};
	struct SkinMeshNode : MeshNodeBase {
		DRef_Skin	skin;
		SkinMeshNode(const JValue& v, const IDataQuery& q);

		static SPtr<SkinMeshNode> Load(const JValue& v, const IDataQuery& q);
		void moveTo(void* dst) override;
		void visit(Visitor& v) const override;
	};

	namespace loader {
		using Node = gltf::loader::Node<
			Node,
			CameraNode,
			SkinMeshNode,
			MeshNode,
			Node
		>;
	}
}
