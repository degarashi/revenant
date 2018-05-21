#pragma once
#include "resource.hpp"
#include "idtag.hpp"
#include "../handle/camera.hpp"
#include "../handle/model.hpp"
#include "../handle/opengl.hpp"
#include "../dc/common.hpp"
#include "gltf/dc_common.hpp"

namespace rev::gltf {

	struct Node :
		Resource,
		IResolvable
	{
		struct Visitor {
			virtual ~Visitor() {}
			virtual void upNode() = 0;
			virtual void addNode(const HTfNode& node) = 0;
			virtual void addMesh(const HPrim& p, const HTech& t, const Name& userName, const RTUParams_SP& rt, dc::JointId id) = 0;
			virtual void addSkinMesh(const HPrim& p, const HTech& t, const Name& userName, const RTUParams_SP& rt, const dc::SkinBindSet_SP& bind) = 0;
			virtual void addCamera(const HCam3& c) = 0;
		};
		using Pose3 = beat::g3::Pose;
		Pose3				pose;
		TagNodeV			child;
		SName				jointName;
		dc::JointId			jointId;
		static dc::JointId	s_id;

		void _visit(Visitor& v) const;
		Node(const JValue& v);
		Type getType() const noexcept override;
		void resolve(const ITagQuery& q) override;
		virtual void visit(Visitor& v) const;
	};
	struct CameraNode : Node {
		TagCamera		camera;

		static std::shared_ptr<CameraNode> Load(const JValue& v);
		CameraNode(const JValue& v);
		void resolve(const ITagQuery& q) override;
		void visit(Visitor& v) const override;
	};
	struct MeshNodeBase : Node {
		TagMeshV		mesh;

		MeshNodeBase(const JValue& v);
		void resolve(const ITagQuery& q) override;
	};
	struct MeshNode : MeshNodeBase {
		static std::shared_ptr<MeshNode> Load(const JValue& v);
		MeshNode(const JValue& v);
		void resolve(const ITagQuery& q) override;
		void visit(Visitor& v) const override;
	};
	struct SkinMeshNode : MeshNodeBase {
		TagSkin			skin;
		TagNodeV		skeleton;

		static std::shared_ptr<SkinMeshNode> Load(const JValue& v);
		SkinMeshNode(const JValue& v);
		void resolve(const ITagQuery& q) override;
		void visit(Visitor& v) const override;
	};

	namespace loader {
		struct Node : Node_SP {
			using value_t = Node_SP;
			Node(const JValue& v);
		};
	}
}
