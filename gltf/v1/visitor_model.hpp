#pragma once
#include "gltf/v1/node.hpp"

namespace rev::gltf::v1 {
	class Visitor_Model : public Node::Visitor {
		private:
			using NodeStack  = std::vector<dc::TfNode*>;
			using MeshV = std::vector<HMesh_U>;

			HTf			_tfRoot;
			NodeStack	_stack;
			MeshV		_mesh,
						_skinmesh;
		public:
			Visitor_Model();
			void upNode() override;
			void addNode(const HTfNode& node) override;
			void addMesh(const HPrim& p, const HTech& t, const Name& userName, const RTUParams_SP& rt, dc::JointId id) override;
			void addSkinMesh(const HPrim& p, const HTech& t, const Name& userName, const RTUParams_SP& rt, const SkinBindSet_SP& bind) override;
			void addCamera(const HCam3& c) override;

			HMdl result() const;
	};
}
