#include "visitor_camera.hpp"

namespace rev::gltf::v1 {
	using VC = Visitor_Camera;
	void VC::upNode() {
		_stack.pop_back();
	}
	void VC::addNode(const Node& node) {
		_stack.emplace_back(&node);
	}
	void VC::addCamera(const HCam3& c) {
		auto* node = _stack.back();
		_camera.emplace_back(
			Ent{
				.camera = c,
				.id = node->jointId,
				.name = node->username ? *node->username : Name("(noname)")
			}
		);
	}
	const VC::CameraV& VC::result() const {
		return _camera;
	}
}
