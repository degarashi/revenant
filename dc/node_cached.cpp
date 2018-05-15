#include "dc/node_cached.hpp"

namespace rev::dc {
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
	Mat4 NodeParam_cached::getLocal(const SName& name) const {
		return _np.getLocal(name);
	}
	Mat4 NodeParam_cached::getGlobal(const SName& name) const {
		return _np.getGlobal(name);
	}
}
