#include "sampler_if.hpp"
#include "frea/quaternion.hpp"

namespace rev::dc {
	template <class T>
	using SVec = std::shared_ptr<std::vector<T>>;

	struct Pose_T_Sampler : IPoseSampler {
		SVec<frea::Vec3>	value;

		std::size_t numKey() const override;
		void sample(Pose3& dst, std::size_t idx, float t) const override;
	};
	struct Pose_R_Sampler : IPoseSampler {
		SVec<frea::Quat>	value;

		std::size_t numKey() const override;
		void sample(Pose3& dst, std::size_t idx, float t) const override;
	};
	struct Pose_S_Sampler : IPoseSampler {
		SVec<frea::Vec3>	value;

		std::size_t numKey() const override;
		void sample(Pose3& dst, std::size_t idx, float t) const override;
	};
}
