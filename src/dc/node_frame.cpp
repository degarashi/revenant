#include "dc/node_frame.hpp"
#include "dc/node.hpp"
#include "frea/src/interpolation.hpp"
#include "beat/src/pose3d.hpp"

namespace rev::dc {
	std::size_t Node_FrameOut::numFrame() const {
		return value->size() / getNUnit();
	}
	const float* Node_FrameOut::_calcValue(const std::size_t idx, const float t) const {
		D_Assert0(lubee::IsInRange(t, 0.f, 1.f));
		const auto NUnit = getNUnit();
		auto& val = *value;

		if(t == 0.f)
			for(std::size_t i=0 ; i<NUnit ; i++)
				tmp[i] = val[idx * NUnit + i];
		else {
			D_Assert0((idx+1)*NUnit < val.size());
			frea::Lerp(
				tmp,
				val.data() + idx*NUnit,
				val.data() + (idx+1)*NUnit,
				val.data() + (idx+1)*NUnit,
				t
			);
		}
		return tmp;
	}

	std::size_t Node_T_FrameOut::getNUnit() const noexcept {
		return 3;
	}
	void Node_T_FrameOut::output(TfNode& dst, const std::size_t idx, const float t) const {
		auto* res = reinterpret_cast<const frea::Vec3*>(_calcValue(idx, t));
		dst.refPose().setOffset(*res);
	}
	std::size_t Node_R_FrameOut::getNUnit() const noexcept {
		return 4;
	}
	void Node_R_FrameOut::output(TfNode& dst, const std::size_t idx, const float t) const {
		D_Assert0(lubee::IsInRange(t, 0.f, 1.f));
		auto& val = *value;

		frea::Quat q;
		if(t == 0.f)
			q = *reinterpret_cast<const frea::Quat*>(val.data() + idx*4);
		else {
			D_Assert0((idx+1)*4 < val.size());
			auto q0 = *reinterpret_cast<const frea::Quat*>(val.data() + idx*4),
				q1 = *reinterpret_cast<const frea::Quat*>(val.data() + (idx+1)*4);
			if(q0.dot(q1) < 0)
				q1 *= -1;
			q = frea::Lerp(q0, q1, t);
		}
		dst.refPose().setRotation(q);
	}
	std::size_t Node_S_FrameOut::getNUnit() const noexcept {
		return 3;
	}
	void Node_S_FrameOut::output(TfNode& dst, const std::size_t idx, const float t) const {
		auto* res = reinterpret_cast<const frea::Vec3*>(_calcValue(idx, t));
		dst.refPose().setScaling(*res);
	}

	std::size_t Node_W_FrameOut::getNUnit() const noexcept {
		return nWeight;
	}
	void Node_W_FrameOut::output(TfNode& dst, const std::size_t idx, const float t) const {
		auto* dsf = static_cast<std::vector<float>*>(dst.userData.get());
		nWeight = dsf->size();
		auto* res = _calcValue(idx, t);
		dsf->assign(res, res+nWeight);
	}
}
