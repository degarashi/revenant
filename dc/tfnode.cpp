#include "dc/node.hpp"

namespace rev::dc {
	void TfNode::OnParentChange(spi::TreeNode<TfNode>* self, const HTfNode&) {
		// Transformの更新をかける
		static_cast<TfNode*>(self)->refPose();
	}
	bool TfNode::_refresh(Transform::value_t& dst, Transform*) const {
		const auto& p = getPose();
		if(const auto parent = getParent()) {
			dst = p.getToWorld().convertI<4,4>(1) * parent->getTransform().convertI<4,4>(1);
		} else
			dst = p.getToWorld().convertI<4,4>(1);
		return true;
	}
	TfNode::TfNode(const JointId id, const SName& jointName, const Name& userName):
		id(id),
		jointName(jointName),
		userName(userName)
	{}
}
