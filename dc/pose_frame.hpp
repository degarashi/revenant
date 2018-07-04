#include "pose_frame_if.hpp"
#include "frea/quaternion.hpp"

namespace beat::g3 {
	class Pose;
}
namespace rev::dc {
	template <class T>
	using SVec = std::shared_ptr<std::vector<T>>;
	using Pose3 = beat::g3::Pose;

	struct Pose_T_Sampler : IFrameOut<Pose3> {
		SVec<frea::Vec3>	value;

		std::size_t numFrame() const override;
		void output(Pose3& dst, std::size_t idx, float t) const override;
	};
	struct Pose_R_Sampler : IFrameOut<Pose3> {
		SVec<frea::Quat>	value;

		std::size_t numFrame() const override;
		void output(Pose3& dst, std::size_t idx, float t) const override;
	};
	struct Pose_S_Sampler : IFrameOut<Pose3> {
		SVec<frea::Vec3>	value;

		std::size_t numFrame() const override;
		void output(Pose3& dst, std::size_t idx, float t) const override;
	};
}
