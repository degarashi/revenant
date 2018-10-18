#include "gltf/v2/animation.hpp"
#include "gltf/v2/node.hpp"
#include "gltf/v2/accessor.hpp"
#include "../value_loader.hpp"
#include "../check.hpp"
#include "../../dc/node_frame.hpp"

namespace rev::gltf::v2 {
	namespace L = gltf::loader;
	// ------------------ AnimSampler ------------------
	namespace {
		using Interpolation = AnimSampler::Interpolation;
		const std::pair<std::string, Interpolation> c_intp[] = {
			{"LINEAR", Interpolation::Linear},
			{"STEP", Interpolation::Step},
			{"CUBICSPLINE", Interpolation::CubicSpline},
		};
		const std::pair<std::string, AnimPath> c_path[] = {
			{"translation", AnimPath::Translation},
			{"rotation", AnimPath::Rotation},
			{"scale", AnimPath::Scale},
			{"weights", AnimPath::Weights},
		};
	}
	AnimSampler::AnimSampler(const JValue& v, const IDataQuery& q):
		Resource(v),
		input(L::Required<DRef_Accessor>(v, "input", q)),
		output(L::Required<DRef_Accessor>(v, "output", q)),
		interpolation(Interpolation::Linear)
	{
		if(const auto& intp = L::Optional<L::String>(v, "interpolation")) {
			const auto& e = CheckEnum(c_intp, *intp, [](auto& c, auto& i){
				return c.first == i;
			});
			interpolation = e.second;
		}
	}
	Resource::Type AnimSampler::getType() const noexcept {
		return Type::AnimSampler;
	}
	const FVec& AnimSampler::_getCache() const {
		if(!cached) {
			cached = std::make_shared<std::vector<float>>(output->cnvToFloat());
		}
		return cached;
	}
	HFrameOut AnimSampler::asFrameOut(const AnimPath type) const {
		const auto& c = _getCache();
		if(type == AnimPath::Translation) {
			const auto samp = std::make_shared<dc::Node_T_FrameOut>();
			samp->value = c;
			return samp;
		} else if(type == AnimPath::Rotation) {
			const auto samp = std::make_shared<dc::Node_R_FrameOut>();
			samp->value = c;
			return samp;
		} else if(type == AnimPath::Scale) {
			const auto samp = std::make_shared<dc::Node_S_FrameOut>();
			samp->value = c;
			return samp;
		} else {
			Assert0(type == AnimPath::Weights);
			const auto samp = std::make_shared<dc::Node_W_FrameOut>();
			samp->value = c;
			return samp;
		}
	}

	// ------------------ Animation::Channel::Target ------------------
	Animation::Channel::Target::Target(const JValue& v, const IDataQuery& q):
		node(L::Optional<DRef_Node>(v, "node", q))
	{
		const auto p = L::Required<L::String>(v, "path");
		const auto& e = CheckEnum(c_path, p, [](auto& c, auto& p){
			return c.first == std::string(p);
		});
		path = e.second;
	}

	// ------------------ Animation::Channel ------------------
	Animation::Channel::Channel(const JValue& v, const IDataQuery& q):
		sampler(L::Required<DRef_AnimSampler>(v, "sampler", q)),
		target(L::Required<Target>(v, "target", q))
	{}
	bool Animation::Channel::CanLoad(const JValue& v) {
		return v.IsObject();
	}

	// ------------------ Animation ------------------
	namespace {
		struct QueryOvr : IDataQuery_Nest {
			const Animation&	_a;
			QueryOvr(const Animation& a, const IDataQuery& q):
				IDataQuery_Nest(q),
				_a(a)
			{}
			const AnimSampler& getAnimSampler(const DataId& id) const noexcept override {
				if(id < _a.sampler.size())
					return _a.sampler[id];
				return IDataQuery_Nest::getAnimSampler(id);
			}
		};
	}
	Animation::Animation(const JValue& v, const IDataQuery& q):
		Resource(v),
		sampler(L::Required<L::Array<AnimSampler>>(v, "samplers", QueryOvr(*this, q))),
		channel(L::Required<L::Array<Channel>>(v, "channels", QueryOvr(*this, q)))
	{}
	Resource::Type Animation::getType() const noexcept {
		return Type::Animation;
	}
}
#include "../../dc/seekframe.hpp"
#include "../../dc/joint_at.hpp"
#include "../../dc/channel.hpp"
namespace rev::gltf::v2 {
	dc::Animation Animation::makeAnimation() const {
		dc::Animation ret;
		for(auto& c : channel) {
			auto ch = std::make_shared<dc::Channel>();
			if(!c.target.node)
				continue;
			// Joint At
			{
				const auto jat = std::make_shared<dc::Jat_Id>();
				jat->id = (*c.target.node)->jointId;
				ch->_jat = jat;
			}
			// Sampler(output) -> (T or R or S or W)_Sampler
			ch->_output = c.sampler->asFrameOut(c.target.path);
			// Sampler(input) -> PosSampler
			{
				auto isamp = std::make_shared<dc::SeekFrame>();
				isamp->pos = std::make_shared<std::vector<float>>(c.sampler->input->cnvToFloat());
				ch->_seek = isamp;
			}
			ret.addChannel(ch);
		}
		return ret;
	}
}
