#include "dc/pose_sampler.hpp"
#include "frea/interpolation.hpp"
#include "beat/pose3d.hpp"

namespace rev::dc {
	std::size_t Pose_T_Sampler::numFrame() const {
		return value->size();
	}
	void Pose_T_Sampler::sample(Pose3& dst, const std::size_t idx, const float t) const {
		D_Assert0(lubee::IsInRange(t, 0.f, 1.f));
		auto& val = *value;
		frea::Vec3 v;
		if(t == 0.f)
			v = val[idx];
		else
			v = frea::Lerp(val[idx], val[idx+1], t);
		dst.setOffset(v);
	}
	std::size_t Pose_R_Sampler::numFrame() const {
		return value->size();
	}
	void Pose_R_Sampler::sample(Pose3& dst, const std::size_t idx, const float t) const {
		D_Assert0(lubee::IsInRange(t, 0.f, 1.f));
		auto& val = *value;
		frea::Quat q;
		if(t == 0.f)
			q = val[idx];
		else
			q = frea::Lerp(val[idx], val[idx+1], t);
		dst.setRotation(q);
	}
	std::size_t Pose_S_Sampler::numFrame() const {
		return value->size();
	}
	void Pose_S_Sampler::sample(Pose3& dst, const std::size_t idx, const float t) const {
		D_Assert0(lubee::IsInRange(t, 0.f, 1.f));
		auto& val = *value;
		frea::Vec3 s;
		if(t == 0.f)
			s = val[idx];
		else
			s = frea::Lerp(val[idx], val[idx+1], t);
		dst.setScaling(s);
	}
}
