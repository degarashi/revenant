#include "node.hpp"
#include "value_loader.hpp"

namespace rev::gltf {
	namespace L = gltf::loader;

	template <class D_Node, class Q>
	Node<D_Node, Q>::Node(const JValue& v, const Q& q):
		pose(gltf::loader::Pose3(v)),
		child(L::OptionalDefault<L::Array<D_Node>>(v, "children", {}, q)),
		jointId(s_id++),
		parent(nullptr)
	{}
	template <class D_Node, class Q>
	void Node<D_Node, Q>::setParent() {
		for(auto& c : child) {
			using T = std::remove_const_t<std::remove_reference_t<decltype(*c)>>;
			const_cast<T&>(*c).parent = this;
		}
	}
	template <class D_Node, class Q>
	dc::JointId Node<D_Node, Q>::s_id = 1;
}

#include "gltf/v1/node.hpp"
namespace rev::gltf {
	template class Node<v1::DRef_Node, v1::IDataQuery>;
}
