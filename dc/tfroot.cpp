#include "dc/node.hpp"

namespace rev::dc {
	bool TfRoot::_refresh(NameToNode::value_t& dst, NameToNode*) const {
		dst.clear();
		const auto& node = getNode();
		for(auto& n : node) {
			n->iterateDepthFirst<true>([&dst](auto& n, int){
				if(const auto& name = n.jointName)
					dst.emplace(*name, &n);
				return spi::Iterate::StepIn;
			});
		}
		return true;
	}
	bool TfRoot::_refresh(IdToNode::value_t& dst, IdToNode*) const {
		dst.clear();
		const auto& node = getNode();
		for(auto& n : node) {
			n->iterateDepthFirst<true>([&dst](auto& n, int){
				dst.emplace(n.id, &n);
				return spi::Iterate::StepIn;
			});
		}
		return true;
	}

	frea::Mat4 TfRoot::getLocal(const JointId id) const {
		return find(id)->getPose().getToWorld().convertI<4,4>(1);
	}
	frea::Mat4 TfRoot::getGlobal(const JointId id) const {
		return find(id)->getTransform();
	}
	frea::Mat4 TfRoot::getLocal(const Name& name) const {
		return find(name)->getPose().getToWorld().convertI<4,4>(1);
	}
	frea::Mat4 TfRoot::getGlobal(const Name& name) const {
		return find(name)->getTransform();
	}
	const TfNode* TfRoot::find(const JointId id) const {
		auto& idton = getIdToNode();
		const auto itr = idton.find(id);
		if(itr != idton.end()) {
			return itr->second;
		}
		return nullptr;
	}
	const TfNode* TfRoot::find(const Name& name) const {
		auto& nton = getNameToNode();
		const auto itr = nton.find(name);
		if(itr != nton.end()) {
			return itr->second;
		}
		return nullptr;
	}

	TfNode& TfRoot::query(const JointId id) const {
		return *const_cast<TfNode*>(find(id));
	}
	TfNode& TfRoot::query(const Name& name) const {
		return *const_cast<TfNode*>(find(name));
	}
}
