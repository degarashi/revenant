#include "dc/node_frame.hpp"
#include "dc/node.hpp"
#include "frea/interpolation.hpp"
#include "beat/pose3d.hpp"

namespace rev::dc {
	std::size_t Node_FrameOut::numFrame() const {
		return value->size() / getNUnit();
	}
	const float* Node_FrameOut::_calcValue(const std::size_t idx, const float t) const {
		D_Assert0(lubee::IsInRange(t, 0.f, 1.f));
		const auto NUnit = getNUnit();
		auto& val = *value;
		D_Assert0((idx+1)*NUnit < val.size());

		if(t == 0.f)
			for(std::size_t i=0 ; i<NUnit ; i++)
				tmp[i] = val[idx * NUnit + i];
		else {
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
		auto* res = reinterpret_cast<const frea::Quat*>(_calcValue(idx, t));
		dst.refPose().setRotation(*res);
	}
	std::size_t Node_S_FrameOut::getNUnit() const noexcept {
		return 3;
	}
	void Node_S_FrameOut::output(TfNode& dst, const std::size_t idx, const float t) const {
		auto* res = reinterpret_cast<const frea::Vec3*>(_calcValue(idx, t));
		dst.refPose().setScaling(*res);
	}
}
