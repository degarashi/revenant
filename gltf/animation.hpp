#pragma once
#include "gltf/resource.hpp"
#include "gltf/idtag.hpp"
#include "dc/animation.hpp"

namespace rev {
	namespace gltf {
		struct Animation :
			Resource,
			IResolvable
		{
			struct Sampler :
				IResolvable,
				Resource
			{
				TagAccessor		input,		//!< フレーム位置(= float)
								output;		//!< Transform(Vec3) or Rotation(Quat) or Scaling(Vec3)

				Sampler(const JValue& v);
				void resolve(const ITagQuery& q) override;
				Type getType() const noexcept override;

				mutable struct {
					bool		vec4;
					Void_SP		output;
				} cached;

				void _checkData() const;
				HSampler outputAsTranslation() const;
				HSampler outputAsRotation() const;
				HSampler outputAsScaling() const;
			};
			using Sampler_SP = std::shared_ptr<Sampler>;
			using SamplerM = std::unordered_map<Tag, Sampler_SP>;
			using TagSamp = IDTag<Sampler, Type::AnimSampler>;

			struct Channel : IResolvable {
				struct Target : IResolvable {
					DefineEnum(TRS,
						(Translation)
						(Rotation)
						(Scale)
					);
					TagNode		node;
					TRS			path;

					Target(const JValue& v);
					void resolve(const ITagQuery& q) override;
				};
				TagSamp		sampler;
				Target		target;

				Channel(const JValue& v);
				void resolve(const ITagQuery& q) override;
				static bool CanLoad(const JValue& v) noexcept;
			};
			using ChannelV = std::vector<Channel>;
			using TagCnv = std::unordered_map<Tag, Tag>;

			SamplerM	sampler;	// Channelから参照
			ChannelV	channel;	// アニメーション本体
			TagCnv		tagCnv;		// resolve()内で使用

			Animation(const JValue& v);
			Type getType() const noexcept override;
			void resolve(const ITagQuery& q) override;

			dc::Animation makeAnimation() const;
		};
	}
}
