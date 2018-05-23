#include "dc/node.hpp"
#include "lubee/hash_combine.hpp"

namespace rev::dc {
	bool TfRoot::_refresh(NameToNode::value_t& dst, NameToNode*) const {
		dst.clear();
		const auto& node = getNode();
		for(auto& n : node) {
			n->iterateDepthFirst<true>([&dst](auto& n, int){
				if(n.jointName) {
					if(!n.jointName->empty())
						dst.emplace(n.jointName, &n);
				}
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
	frea::Mat4 TfRoot::getLocal(const SName& name) const {
		return find(name)->getPose().getToWorld().convertI<4,4>(1);
	}
	frea::Mat4 TfRoot::getGlobal(const SName& name) const {
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
	const TfNode* TfRoot::find(const SName& name) const {
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
	TfNode& TfRoot::query(const SName& name) const {
		return *const_cast<TfNode*>(find(name));
	}
}
