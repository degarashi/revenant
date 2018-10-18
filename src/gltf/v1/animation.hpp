#pragma once
#include "gltf/v1/resource.hpp"
#include "gltf/v1/dataref.hpp"
#include "dc/animation.hpp"

namespace rev::gltf::v1 {
	DefineEnum(AnimPath,
		(Translation)
		(Rotation)
		(Scale)
	);
	using FVec = std::shared_ptr<std::vector<float>>;

	struct AnimSampler :
		Resource
	{
		DRef_Accessor	input,		//!< フレーム位置(= float)
						output;		//!< Transform(Vec3) or Rotation(Quat) or Scaling(Vec3)

		AnimSampler(const JValue& v, const IDataQuery& q);
		Type getType() const noexcept override;

		mutable FVec	cached;
		mutable bool	bVec4;
		const FVec& _getCache() const;

		HFrameOut asFrameOut(AnimPath type) const;
	};
	struct Animation :
		Resource
	{
		using SamplerM = std::unordered_map<Tag, AnimSampler>;

		struct Channel {
			struct Target {
				DRef_Node		node;
				AnimPath		path;

				Target(const JValue& v, const IDataQuery& q);
			};
			DRef_AnimSampler	sampler;
			Target				target;

			Channel(const JValue& v, const IDataQuery& q);
			static bool CanLoad(const JValue& v) noexcept;
		};
		using ChannelV = std::vector<Channel>;
		using TagCnv = std::unordered_map<Tag, Tag>;

		TagCnv		tagCnv;		// ctor内で使用
		SamplerM	sampler;	// Channelから参照
		ChannelV	channel;	// アニメーション本体

		Animation(const JValue& v, const IDataQuery& q);
		Type getType() const noexcept override;

		dc::Animation makeAnimation() const;
	};
}
