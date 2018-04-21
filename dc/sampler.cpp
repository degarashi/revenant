#include "dc/sampler.hpp"
#include "frea/interpolation.hpp"
#include "beat/pose3d.hpp"

namespace rev::dc {
	std::size_t T_Sampler::numKey() const {
		return value->size();
	}
	void T_Sampler::sample(Pose3& dst, const std::size_t idx0, const std::size_t idx1, const float t) const {
		D_Assert0(idx0 <= idx1);
		D_Assert0(lubee::IsInRange(t, 0.f, 1.f));
		auto& val = *value;
		frea::Vec3 v;
		if(idx0 == idx1)
			v = val[idx0];
		else
			v = frea::Lerp(val[idx0], val[idx1], t);
		dst.setOffset(v);
	}
	std::size_t R_Sampler::numKey() const {
		return value->size();
	}
	void R_Sampler::sample(Pose3& dst, const std::size_t idx0, const std::size_t idx1, const float t) const {
		D_Assert0(idx0 <= idx1);
		D_Assert0(lubee::IsInRange(t, 0.f, 1.f));
		auto& val = *value;
		frea::Quat q;
		if(idx0 == idx1)
			q = val[idx0];
		else
			q = frea::Lerp(val[idx0], val[idx1], t);
		dst.setRotation(q);
	}
	std::size_t S_Sampler::numKey() const {
		return value->size();
	}
	void S_Sampler::sample(Pose3& dst, const std::size_t idx0, const std::size_t idx1, const float t) const {
		D_Assert0(idx0 <= idx1);
		D_Assert0(lubee::IsInRange(t, 0.f, 1.f));
		auto& val = *value;
		frea::Vec3 s;
		if(idx0 == idx1)
			s = val[idx0];
		else
			s = frea::Lerp(val[idx0], val[idx1], t);
		dst.setScaling(s);
	}
}
