#pragma once
#include "gltf/node.hpp"

namespace rev::gltf {
	using Name = std::string;
	class NodeVisitor_CameraBase {
		public:
			struct Ent {
				HCam3			camera;
				dc::JointId		id;
				Name			name;
			};
			using CameraV = std::vector<Ent>;
	};
	template <class N>
	class NodeVisitor_Camera :
		public NodeVisitor<N>,
		public NodeVisitor_CameraBase
	{
		private:
			using NodeStack = std::vector<const N*>;
			NodeStack	_stack;
			CameraV		_camera;
		public:
			void upNode() override {
				_stack.pop_back();
			}
			void addNode(const N& node) override {
				_stack.emplace_back(&node);
			}
			void addCamera(const HCam3& c) override {
				auto* node = this->_stack.back();
				_camera.emplace_back(
					Ent{
						.camera = c,
						.id = node->jointId,
						.name = node->username ? *node->username : Name("(noname)")
					}
				);
			}
			const CameraV& result() const {
				return _camera;
			}
	};
}
