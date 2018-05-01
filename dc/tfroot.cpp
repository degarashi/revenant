#include "dc/node.hpp"

namespace rev::dc {
	bool SkinBind::operator == (const SkinBind& bind) const noexcept {
		return jointName == bind.jointName &&
			invmat == bind.invmat;
	}

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
	const Mat4V& TfRoot::getJointMat(const Mat4&, const SkinBindV_SP&, const Mat4&) const {
		Assert(false, "invaid function called");
		throw 0;
	}

	// ---------------------- NodeParam_cached::MatP ----------------------
	std::size_t NodeParam_cached::MatP::hash() const noexcept {
		const std::hash<Mat4> h;
		return h(node_m) + h(bs_m);
	}
	bool NodeParam_cached::MatP::operator == (const MatP& m) const noexcept {
		return node_m == m.node_m &&
			bs_m == m.bs_m;
	}
	// ---------------------- NodeParam_cached::KeyS ----------------------
	std::size_t NodeParam_cached::KeyS::hash() const noexcept {
		return mat.hash() + std::hash<SkinBind>()(bind);
	}
	bool NodeParam_cached::KeyS::operator == (const KeyS& m) const noexcept {
		return mat==m.mat && bind==m.bind;
	}
	// ---------------------- NodeParam_cached::KeyV ----------------------
	std::size_t NodeParam_cached::KeyV::hash() const noexcept {
		return mat.hash() + std::hash<SkinBindV_SP>()(bind);
	}
	bool NodeParam_cached::KeyV::operator == (const KeyV& m) const noexcept {
		return mat==m.mat && bind==m.bind;
	}
	// ---------------------- NodeParam_cached ----------------------
	NodeParam_cached::NodeParam_cached(const NodeParam& np):
		_np(np)
	{}
	const Mat4& NodeParam_cached::_getSingle(const MatP& mat, const SkinBind& bind) const {
		const KeyS ks{mat, bind};
		auto itr = _single.find(ks);
		if(itr == _single.end()) {
			const Mat4 m = mat.node_m.transposition() * getGlobal(*bind.jointName).transposition() * bind.invmat * mat.bs_m;
			itr = _single.emplace(ks, m).first;
		}
		return itr->second;
	}
	const Mat4V& NodeParam_cached::getJointMat(const Mat4& node_m, const SkinBindV_SP& bind, const Mat4& bs_m) const {
		const MatP mp{node_m, bs_m};
		const KeyV kv{mp, bind};
		const auto itr = _vec.find(kv);
		if(itr != _vec.end()) {
			return itr->second;
		}

		const auto len = bind->size();
		Mat4V mv(len);
		for(std::size_t i=0 ; i<len ; i++) {
			mv[i] = _getSingle(mp, (*bind)[i]);
		}
		return _vec.emplace(kv, std::move(mv)).first->second;
	}
	Mat4 NodeParam_cached::getLocal(const JointId id) const {
		return _np.getLocal(id);
	}
	Mat4 NodeParam_cached::getGlobal(const JointId id) const {
		return _np.getGlobal(id);
	}
	Mat4 NodeParam_cached::getLocal(const Name& name) const {
		return _np.getLocal(name);
	}
	Mat4 NodeParam_cached::getGlobal(const Name& name) const {
		return _np.getGlobal(name);
	}
}
