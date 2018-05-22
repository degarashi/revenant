#include "dc/skin_cached.hpp"
#include "lubee/hash_combine.hpp"

namespace rev::dc {
	std::size_t QueryMatrix_skincached::KeyS::operator ()(const KeyS& k) const noexcept {
		return lubee::hash_combine_implicit(k.bind, bs_m);
	}
	bool QueryMatrix_skincached::KeyS::operator == (const KeyS& k) const noexcept {
		return bind == k.bind &&
				bs_m == k.bs_m;
	}

	QueryMatrix_skincached::QueryMatrix_skincached(const IQueryMatrix& qm):
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
		_qm(qm),
		_jointId(~0)
	{}
	void QueryMatrix_skincached::setNodeJointId(const JointId id) {
		if(_jointId != id) {
			_jointId = id;
			_node_m = _qm.getGlobal(id);
			_single.clear();
			_vec.clear();
		}
	}
	QueryMatrix_skincached::QueryMatrix_skincached(const QueryMatrix_skincached& qm):
		QueryMatrix_skincached(qm._qm)
	{
		setNodeJointId(qm._jointId);
	}
	const Mat4V& QueryMatrix_skincached::getJointMat(const Mat4&, const SkinBindSet_SP& bind) const {
		return _vec.getCache(bind);
	}
	Mat4 QueryMatrix_skincached::getLocal(const JointId id) const {
		return _qm.getLocal(id);
	}
	Mat4 QueryMatrix_skincached::getGlobal(const JointId id) const {
		return _qm.getGlobal(id);
	}
	Mat4 QueryMatrix_skincached::getLocal(const SName& name) const {
		return _qm.getLocal(name);
	}
	Mat4 QueryMatrix_skincached::getGlobal(const SName& name) const {
		return _qm.getGlobal(name);
	}
}
