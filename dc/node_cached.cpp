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
		_single([this](const KeyS& k) -> Mat4{
			return k.mat.node_m.transposition() * getGlobal(*k.bind.jointName).transposition() * k.bind.invmat * k.mat.bs_m;
		}),
		_vec([this](const KeyV& k) -> Mat4V{
			const auto len = k.bind->size();
			Mat4V mv(len);
			for(std::size_t i=0 ; i<len ; i++) {
				mv[i] = _single.getCache(KeyS{k.mat, (*k.bind)[i]});
			}
			return mv;
		}),
		_np(np)
	{}
	NodeParam_cached::NodeParam_cached(const NodeParam_cached& np):
		NodeParam_cached(np._np)
	{}
	const Mat4V& NodeParam_cached::getJointMat(const Mat4& node_m, const SkinBindV_SP& bind, const Mat4& bs_m) const {
		const MatP mp{node_m, bs_m};
		const KeyV k{mp, bind};
		return _vec.getCache(k);
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
