#include "frameout_if.hpp"
#include "frea/quaternion.hpp"

namespace rev::dc {
	template <class T>
	using SVec = std::shared_ptr<std::vector<T>>;

	struct Pose_T_Sampler : IFrameOut {
		SVec<frea::Vec3>	value;

		std::size_t numFrame() const override;
		void output(TfNode& dst, std::size_t idx, float t) const override;
	};
	struct Pose_R_Sampler : IFrameOut {
		SVec<frea::Quat>	value;

		std::size_t numFrame() const override;
		void output(TfNode& dst, std::size_t idx, float t) const override;
	};
	struct Pose_S_Sampler : IFrameOut {
		SVec<frea::Vec3>	value;

		std::size_t numFrame() const override;
		void output(TfNode& dst, std::size_t idx, float t) const override;
	};
}
