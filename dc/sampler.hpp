#include "sampler_if.hpp"
#include "frea/quaternion.hpp"

namespace rev::dc {
	template <class T>
	using SVec = std::shared_ptr<std::vector<T>>;

	struct T_Sampler : ISampler {
		SVec<frea::Vec3>	value;

		std::size_t numKey() const override;
		void sample(Pose3& dst, std::size_t idx0, std::size_t idx1, float t) const override;
	};
	struct R_Sampler : ISampler {
		SVec<frea::Quat>	value;

		std::size_t numKey() const override;
		void sample(Pose3& dst, std::size_t idx0, std::size_t idx1, float t) const override;
	};
	struct S_Sampler : ISampler {
		SVec<frea::Vec3>	value;

		std::size_t numKey() const override;
		void sample(Pose3& dst, std::size_t idx0, std::size_t idx1, float t) const override;
	};
}
