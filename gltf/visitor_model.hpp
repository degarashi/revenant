#pragma once
#include "gltf/node.hpp"

namespace rev::gltf {
	class Visitor_Model : public Node::Visitor {
		private:
			using NodeStack  = std::vector<dc::TfNode*>;
			using MeshV = std::vector<HMesh>;

			HTf			_tfRoot;
			NodeStack	_stack;
			MeshV		_mesh;
		public:
			Visitor_Model();
			void upNode() override;
			void addNode(const HTfNode& node) override;
			void addMesh(const HPrim& p, const HTech& t, const Name& userName, const RTUParams_SP& rt, dc::JointId id) override;
			void addSkinMesh(const HPrim& p, const HTech& t, const Name& userName, const RTUParams_SP& rt, const dc::SkinBindV_SP& bind, const frea::Mat4& bsm) override;
			void addCamera(const HCam3& c) override;

			HMdl result() const;
	};
}
