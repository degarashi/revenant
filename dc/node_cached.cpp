#include "dc/node_cached.hpp"
#include "lubee/hash_combine.hpp"

namespace rev::dc {
	std::size_t NodeParam_cached::KeyS::operator ()(const KeyS& k) const noexcept {
		return lubee::hash_combine_implicit(k.bind, bs_m);
	}
	bool NodeParam_cached::KeyS::operator == (const KeyS& k) const noexcept {
		return bind == k.bind &&
				bs_m == k.bs_m;
	}

	NodeParam_cached::NodeParam_cached(const NodeParam& np):
		_single([this](const KeyS& s) -> Mat4{
			return _node_m.transposition() * getGlobal(*s.bind.jointName).transposition() * s.bind.invmat * s.bs_m;
		}),
		_vec([this](const SkinBindSet_SP& s) -> Mat4V{
			const auto len = s->bind.size();
			Mat4V mv(len);
			for(std::size_t i=0 ; i<len ; i++) {
				mv[i] = _single.getCache(KeyS{s->bind[i], s->bs_m});
			}
			return mv;
		}),
		_np(np),
		_jointId(~0)
	{}
	void NodeParam_cached::setNodeJointId(const JointId id) {
		if(_jointId != id) {
			_jointId = id;
			_node_m = _np.getGlobal(id);
			_single.clear();
			_vec.clear();
		}
	}
	NodeParam_cached::NodeParam_cached(const NodeParam_cached& np):
		NodeParam_cached(np._np)
	{
		setNodeJointId(np._jointId);
	}
	const Mat4V& NodeParam_cached::getJointMat(const Mat4&, const SkinBindSet_SP& bind) const {
		return _vec.getCache(bind);
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
