#pragma once
#include "gltf/v2/node.hpp"
#include "gltf/v2/dc_mesh_if.hpp"
#include "../../handle/model.hpp"

namespace rev::gltf::v2 {
	class Visitor_Model : public Node::Visitor {
		private:
			using NodeStack  = std::vector<dc::TfNode*>;
			using MeshV = std::vector<HGMesh>;

			HTf			_tfRoot;
			NodeStack	_stack;
			MeshV		_mesh,
						_skinmesh;
		public:
			Visitor_Model();
			void upNode() override;
			void addNode(const Node& node) override;
			void addMesh(const HGMesh& m) override;
			void addSkinMesh(const HGMesh& m) override;

			HMdl result() const;
	};
}
