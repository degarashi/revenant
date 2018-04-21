#include "dc/node.hpp"

namespace rev::dc {
	bool TfRoot::_refresh(NameToNode::value_t& dst, NameToNode*) const {
		dst.clear();
		const auto& node = getNode();
		for(auto& n : node) {
			n->iterateDepthFirst<true>([&dst](auto& n, int){
				if(const auto& name = n.jointName)
					dst.emplace_back(Name_Node{name.get(), &n});
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
				dst.emplace_back(Id_Node{n.id, &n});
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
		const auto itr = std::find_if(idton.begin(), idton.end(), [id](const auto& n){
			return n.id == id;
		});
		if(itr != idton.end()) {
			return itr->node;
		}
		return nullptr;
	}
	const TfNode* TfRoot::find(const Name& name) const {
		auto& nton = getNameToNode();
		const auto itr = std::find_if(nton.begin(), nton.end(), [&name](const auto& n){
			return *n.name == name;
		});
		if(itr != nton.end()) {
			return itr->node;
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
