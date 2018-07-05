#include "dc/node_frame.hpp"
#include "dc/node.hpp"
#include "frea/interpolation.hpp"
#include "beat/pose3d.hpp"

namespace rev::dc {
	std::size_t Node_FrameOut::numFrame() const {
		return value->size() / getNUnit();
	}
	template <class Res>
	Res Node_FrameOut::_calcValue(const std::size_t idx, const float t) const {
		D_Assert0(lubee::IsInRange(t, 0.f, 1.f));
		const auto NUnit = getNUnit();
		auto& val = *value;
		D_Assert0((idx+1)*NUnit < val.size());

		const auto getElem = [vptr = val.data(), NUnit](const std::size_t n){
			return *reinterpret_cast<const Res*>(vptr + n * NUnit);
		};
		if(t == 0.f)
			return getElem(idx);
		else
			return frea::Lerp(getElem(idx), getElem(idx+1), t);
	}

	std::size_t Node_T_FrameOut::getNUnit() const noexcept {
		return 3;
	}
	void Node_T_FrameOut::output(TfNode& dst, const std::size_t idx, const float t) const {
		const auto res = _calcValue<frea::Vec3>(idx, t);
		dst.refPose().setOffset(res);
	}
	std::size_t Node_R_FrameOut::getNUnit() const noexcept {
		return 4;
	}
	void Node_R_FrameOut::output(TfNode& dst, const std::size_t idx, const float t) const {
		const auto res = _calcValue<frea::Quat>(idx, t);
		dst.refPose().setRotation(res);
	}
	std::size_t Node_S_FrameOut::getNUnit() const noexcept {
		return 3;
	}
	void Node_S_FrameOut::output(TfNode& dst, const std::size_t idx, const float t) const {
		const auto res = _calcValue<frea::Vec3>(idx, t);
		dst.refPose().setScaling(res);
	}
}
