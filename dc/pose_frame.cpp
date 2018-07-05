#include "dc/pose_frame.hpp"
#include "dc/node.hpp"
#include "frea/interpolation.hpp"
#include "beat/pose3d.hpp"

namespace rev::dc {
	std::size_t Pose_T_Sampler::numFrame() const {
		return value->size() / NUnit;
	}
	void Pose_T_Sampler::output(TfNode& dst, const std::size_t idx, const float t) const {
		D_Assert0(lubee::IsInRange(t, 0.f, 1.f));
		auto& val = *value;
		D_Assert0((idx+1)*NUnit < val.size());

		const auto getElem = [vptr = val.data()](const std::size_t n){
			return *reinterpret_cast<const frea::Vec3*>(vptr + n * NUnit);
		};
		frea::Vec3 v;
		if(t == 0.f)
			v = getElem(idx);
		else
			v = frea::Lerp(getElem(idx), getElem(idx+1), t);
		dst.refPose().setOffset(v);
	}
	std::size_t Pose_R_Sampler::numFrame() const {
		return value->size() / NUnit;
	}
	void Pose_R_Sampler::output(TfNode& dst, const std::size_t idx, const float t) const {
		D_Assert0(lubee::IsInRange(t, 0.f, 1.f));
		auto& val = *value;
		D_Assert0((idx+1)*NUnit < val.size());

		const auto getElem = [vptr = val.data()](const std::size_t n){
			return *reinterpret_cast<const frea::Vec4*>(vptr + n * NUnit);
		};
		frea::Quat q;
		if(t == 0.f)
			q = getElem(idx);
		else
			q = frea::Lerp(getElem(idx), getElem(idx+1), t);
		dst.refPose().setRotation(q);
	}
	std::size_t Pose_S_Sampler::numFrame() const {
		return value->size() / NUnit;
	}
	void Pose_S_Sampler::output(TfNode& dst, const std::size_t idx, const float t) const {
		D_Assert0(lubee::IsInRange(t, 0.f, 1.f));
		auto& val = *value;
		D_Assert0((idx+1)*NUnit < val.size());

		const auto getElem = [vptr = val.data()](const std::size_t n){
			return *reinterpret_cast<const frea::Vec3*>(vptr + n * NUnit);
		};
		frea::Vec3 s;
		if(t == 0.f)
			s = getElem(idx);
		else
			s = frea::Lerp(getElem(idx), getElem(idx+1), t);
		dst.refPose().setScaling(s);
	}
}
