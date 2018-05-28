#pragma once
#include "gltf/v1/resource.hpp"
#include "gltf/v1/dataref.hpp"
#include "gltf/v1/dc_common.hpp"
#include "../../handle/camera.hpp"
#include "../../handle/model.hpp"
#include "../../handle/opengl.hpp"

namespace rev::gltf::v1 {
	using DRef_NodeV = std::vector<DRef_Node>;
	struct Node :
		Resource
	{
		struct Visitor {
			virtual ~Visitor() {}
			virtual void upNode() = 0;
			virtual void addNode(const HTfNode& node) = 0;
			virtual void addMesh(const HPrim& p, const HTech& t, const Name& userName, const RTUParams_SP& rt, dc::JointId id) = 0;
			virtual void addSkinMesh(const HPrim& p, const HTech& t, const Name& userName, const RTUParams_SP& rt, const SkinBindSet_SP& bind) = 0;
			virtual void addCamera(const HCam3& c) = 0;
		};
		using Pose3 = beat::g3::Pose;

		Pose3				pose;
		DRef_NodeV			child;
		SName				jointName;
		dc::JointId			jointId;
		static dc::JointId	s_id;

		void _visit(Visitor& v) const;
		Node(const JValue& v, const IDataQuery& q);
		Type getType() const noexcept override;
		virtual void visit(Visitor& v) const;
		virtual void moveTo(void* dst);
	};
	struct CameraNode : Node {
		DRef_Camera		camera;

		static std::shared_ptr<CameraNode> Load(const JValue& v, const IDataQuery& q);
		CameraNode(const JValue& v, const IDataQuery& q);
		void visit(Visitor& v) const override;
		void moveTo(void* dst) override;
	};
	struct MeshNodeBase : Node {
		using DRef_MeshV = std::vector<DRef_Mesh>;
		DRef_MeshV		mesh;

		MeshNodeBase(const JValue& v, const IDataQuery& q);
	};
	struct MeshNode : MeshNodeBase {
		static std::shared_ptr<MeshNode> Load(const JValue& v, const IDataQuery& q);
		MeshNode(const JValue& v, const IDataQuery& q);
		void visit(Visitor& v) const override;
		void moveTo(void* dst) override;
	};
	struct SkinMeshNode : MeshNodeBase {
		DRef_Skin		skin;
		DRef_NodeV		skeleton;

		static std::shared_ptr<SkinMeshNode> Load(const JValue& v, const IDataQuery& q);
		SkinMeshNode(const JValue& v, const IDataQuery& q);
		void visit(Visitor& v) const override;
		void moveTo(void* dst) override;
	};

	using Node_SP = std::shared_ptr<Node>;
	namespace loader {
		struct Node : Node_SP {
			using value_t = Node_SP;
			Node(const JValue& v, const IDataQuery& q);
		};
	}
}
