#pragma once
#include "gltf/v1/node.hpp"

namespace rev::gltf::v1 {
	class Visitor_Camera : public Node::Visitor {
		public:
			struct Ent {
				HCam3			camera;
				dc::JointId		id;
				Name			name;
			};
			using CameraV = std::vector<Ent>;
		private:
			using NodeStack = std::vector<const Node*>;
			CameraV		_camera;
			NodeStack	_stack;
		public:
			void upNode() override;
			void addNode(const Node& node) override;
			void addCamera(const HCam3& c) override;
			const CameraV& result() const;
	};
}
