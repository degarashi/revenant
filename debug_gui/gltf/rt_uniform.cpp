#include "../../gltf/technique.hpp"
#include "../print.hpp"
#include "../entry_field.hpp"
#include "../../gltf/node.hpp"

namespace rev::gltf {
	const char* Technique::UnifParam_Sem::getDebugName() const noexcept {
		return "UnifParam_Sem";
	}
	const char* Technique::UnifParam_JointMat::getDebugName() const noexcept {
		return "UnifParam_JointMat";
	}
	const char* Technique::UnifParam_NodeSem::getDebugName() const noexcept {
		return "UnifParam_NodeSem";
	}

	bool Technique::UnifParam_Sem::property(const bool edit) {
		auto f = debug::EntryField(nullptr, edit, 2);
		f.show("Type", getDebugName());
		f.show("Semantic", semantic);
		return f.modified();
	}
	bool Technique::UnifParam_JointMat::property(const bool edit) {
		auto f = debug::EntryField(nullptr, edit, 2);
		f.show("Type", getDebugName());
		f.show("Count", count);
		return f.modified();
	}
	bool Technique::UnifParam_NodeSem::property(const bool edit) {
		auto f = debug::EntryField(nullptr, edit, 2);
		f.show("Type", getDebugName());
		f.show("Semantic", semantic);
		f.show("Node-JointId", node->jointId);
		f.show("Node-Name", node->jointName->c_str());
		return f.modified();
	}
}
