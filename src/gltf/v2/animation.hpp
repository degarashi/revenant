#pragma once
#include "gltf/v2/resource.hpp"
#include "gltf/v2/dataref.hpp"
#include "../../handle/model.hpp"
#include "../../dc/animation.hpp"

namespace rev::gltf::v2 {
	DefineEnum(AnimPath,
		(Translation)
		(Rotation)
		(Scale)
		(Weights)
	);
	using FVec = std::shared_ptr<std::vector<float>>;

	class AnimSampler :
		public Resource
	{
		public:
			DefineEnum(Interpolation,
				(Linear)
				(Step)
				(CubicSpline)
			);
			DRef_Accessor	input,
							output;
			Interpolation	interpolation;		// Default: Linear

			AnimSampler(const JValue& v, const IDataQuery& q);

			mutable FVec	cached;
			const FVec& _getCache() const;

			HFrameOut asFrameOut(AnimPath type) const;
			Type getType() const noexcept override;
	};
	class Animation :
		public Resource
	{
		public:
			struct Channel {
				struct Target {
					DRef_Node_OP	node;
					AnimPath		path;

					Target(const JValue& v, const IDataQuery& q);
				};
				DRef_AnimSampler	sampler;
				Target				target;

				Channel(const JValue& v, const IDataQuery& q);
				static bool CanLoad(const JValue& v);
			};
			using ChannelV = std::vector<Channel>;
			using AnimSamplerV = std::vector<AnimSampler>;

			AnimSamplerV	sampler;
			ChannelV		channel;

			Animation(const JValue& v, const IDataQuery& q);
			Type getType() const noexcept override;

			dc::Animation makeAnimation() const;
	};
}
