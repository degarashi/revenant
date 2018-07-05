#include "gltf/v1/animation.hpp"
#include "../check.hpp"
#include "gltf/v1/accessor.hpp"
#include "dc/node_frame.hpp"
#include "../../ovr_functor.hpp"

namespace rev::gltf::v1 {
	using namespace loader;
	// ---------------------- AnimSampler ----------------------
	AnimSampler::AnimSampler(const JValue& v, const IDataQuery& q):
		Resource(v),
		input(Required<DRef_Accessor>(v, "input", q)),
		output(Required<DRef_Accessor>(v, "output", q))
	{
		if(!v.IsObject())
			throw InvalidProperty("not an object");
		{
			const auto ip = OptionalDefault<String>(v, "interpolation", "LINEAR");
			if(std::strcmp(ip, "LINEAR") != 0)
				throw InvalidEnum("");
		}
	}
	Resource::Type AnimSampler::getType() const noexcept {
		return Type::AnimSampler;
	}
	namespace {
		template <class T>
		using Vec = std::vector<T>;
	}
	const FVec& AnimSampler::_getCache() const {
		if(!cached) {
			output->getData(
				OVR_Functor{
					[this](const frea::Vec3* v, const auto len) {
						auto* fp = reinterpret_cast<const float*>(v);
						cached = std::make_shared<std::vector<float>>(fp, fp+len*3);
						bVec4 = false;
					},
					[this](const frea::Vec4* v, const auto len) {
						auto* fp = reinterpret_cast<const float*>(v);
						cached = std::make_shared<std::vector<float>>(fp, fp+len*4);
						bVec4 = true;
					},
					[](const auto&, auto) {
						AssertF("output type must Vec3 or Vec4");
					},
				}
			);
		}
		return cached;
	}
	HFrameOut AnimSampler::asFrameOut(const AnimPath type) const {
		const auto& c = _getCache();
		if(type == AnimPath::Translation) {
			Assert0(!bVec4);
			const auto samp = std::make_shared<dc::Node_T_FrameOut>();
			samp->value = c;
			return samp;
		} else if(type == AnimPath::Rotation) {
			Assert0(bVec4);
			const auto samp = std::make_shared<dc::Node_R_FrameOut>();
			samp->value = c;
			return samp;
		} else {
			Assert0(!bVec4 && type == AnimPath::Scale);
			const auto samp = std::make_shared<dc::Node_S_FrameOut>();
			samp->value = c;
			return samp;
		}
	}

	namespace {
		const std::string c_path[] = {
			"translation",
			"rotation",
			"scale"
		};
	}
	// ---------------------- Animation::Channel::Target ----------------------
	Animation::Channel::Target::Target(const JValue& v, const IDataQuery& q):
		node(Required<DRef_Node>(v, "id", q))
	{
		path = static_cast<AnimPath::e>(&CheckEnum(c_path, static_cast<const char*>(Required<String>(v, "path"))) - c_path);
	}
	// ---------------------- Animation::Channel ----------------------
	Animation::Channel::Channel(const JValue& v, const IDataQuery& q):
		sampler(Required<DRef_AnimSampler>(v, "sampler", q)),
		target(Required<Target>(v, "target", q))
	{}
	bool Animation::Channel::CanLoad(const JValue&) noexcept {
		return true;
	}
	namespace {
		struct QueryOvr : IDataQuery_Nest {
			const Animation&	_a;
			QueryOvr(const Animation& a, const IDataQuery& q):
				IDataQuery_Nest(q),
				_a(a)
			{}
			const Accessor& getAccessor(const Tag& tag) const noexcept override {
				if(const auto itr = _a.tagCnv.find(tag);
					itr != _a.tagCnv.end())
					return IDataQuery_Nest::getAccessor(itr->second);
				return IDataQuery_Nest::getAccessor(tag);
			}
			const AnimSampler& getAnimSampler(const Tag& tag) const noexcept override {
				if(const auto itr = _a.sampler.find(tag);
					itr != _a.sampler.end())
					return itr->second;
				return IDataQuery_Nest::getAnimSampler(tag);
			}
		};
	}
	// ---------------------- Animation ----------------------
	Animation::Animation(const JValue& v, const IDataQuery& q):
		Resource(v),
		tagCnv(OptionalDefault<Dictionary<StdString>>(v, "parameters", {})),
		sampler(OptionalDefault<Dictionary<AnimSampler>>(v, "samplers", {}, QueryOvr(*this, q))),
		channel(OptionalDefault<Array<Channel>>(v, "channels", {}, QueryOvr(*this, q)))
	{}
	Resource::Type Animation::getType() const noexcept {
		return Type::Animation;
	}
}
#include "dc/seekframe.hpp"
#include "dc/channel.hpp"
#include "dc/joint_at.hpp"
#include "gltf/v1/node.hpp"
namespace rev::gltf::v1 {
	dc::Animation Animation::makeAnimation() const {
		dc::Animation ret;
		for(auto& c : channel) {
			auto ch = std::make_shared<dc::Channel>();
			// Joint At
			{
				const auto jat = std::make_shared<dc::Jat_Id>();
				jat->id = c.target.node->jointId;
				ch->_jat = jat;
			}
			// Sampler(output) -> (T or R or S)_Sampler
			ch->_output = c.sampler->asFrameOut(c.target.path);
			// Sampler(input) -> SeekFrame
			{
				auto isamp = std::make_shared<dc::SeekFrame_cached>();
				isamp->pos = std::make_shared<std::vector<float>>(c.sampler->input->cnvToFloat());
				ch->_seek = isamp;
			}
			ret.addChannel(ch);
		}
		return ret;
	}
}
