#include "gltf/v1/animation.hpp"
#include "gltf/v1/check.hpp"
#include "gltf/v1/accessor.hpp"
#include "dc/sampler.hpp"

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
		using SVec = std::shared_ptr<std::vector<T>>;
	}
	void AnimSampler::_checkData() const {
		if(!cached.output) {
			output->getData(
				OVR_Functor{
					[this](const SVec<frea::Vec3>& v) {
						cached.vec4 = false;
						cached.output = v;
					},
					[this](const SVec<frea::Vec4>& v) {
						cached.vec4 = true;
						cached.output = v;
					},
					[](const auto&) {
						AssertF("output type must Vec3 or Vec4");
					},
				}
			);
		}
	}
	HSampler AnimSampler::outputAsTranslation() const {
		_checkData();
		Assert0(!cached.vec4);
		const auto samp = std::make_shared<dc::T_Sampler>();
		samp->value = std::static_pointer_cast<std::vector<frea::Vec3>>(cached.output);
		return samp;
	}
	HSampler AnimSampler::outputAsRotation() const {
		_checkData();
		Assert0(cached.vec4);
		const auto samp = std::make_shared<dc::R_Sampler>();
		samp->value = std::static_pointer_cast<std::vector<frea::Quat>>(cached.output);
		return samp;
	}
	HSampler AnimSampler::outputAsScaling() const {
		_checkData();
		Assert0(!cached.vec4);
		const auto samp = std::make_shared<dc::S_Sampler>();
		samp->value = std::static_pointer_cast<std::vector<frea::Vec3>>(cached.output);
		return samp;
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
		path = static_cast<TRS::e>(&CheckEnum(c_path, static_cast<const char*>(Required<String>(v, "path"))) - c_path);
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
#include "dc/pos_sampler.hpp"
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
			switch(c.target.path) {
				case Channel::Target::TRS::Translation:
					ch->_sampler = c.sampler->outputAsTranslation();
					break;
				case Channel::Target::TRS::Rotation:
					ch->_sampler = c.sampler->outputAsRotation();
					break;
				case Channel::Target::TRS::Scale:
					ch->_sampler = c.sampler->outputAsScaling();
					break;
				default:
					Assert0(false);
			}
			// Sampler(input) -> PosSampler
			{
				auto isamp = std::make_shared<dc::PosSampler_cached>();
				isamp->pos = c.sampler->input->template getDataAs<float>();
				ch->_position = isamp;
			}
			ret.addChannel(ch);
		}
		return ret;
	}
}
