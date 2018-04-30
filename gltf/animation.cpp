#include "gltf/animation.hpp"
#include "gltf/check.hpp"
#include "gltf/accessor.hpp"
#include "dc/sampler.hpp"

namespace rev {
	namespace gltf {
		namespace {
			const std::string c_path[] = {
				"translation",
				"rotation",
				"scale"
			};
		}
		using namespace loader;
		// ---------------------- Animation::Channel::Target ----------------------
		Animation::Channel::Target::Target(const JValue& v):
			node(Required<TagNode>(v, "id"))
		{
			path = static_cast<TRS::e>(&CheckEnum(c_path, static_cast<const char*>(Required<String>(v, "path"))) - c_path);
		}
		void Animation::Channel::Target::resolve(const ITagQuery& q) {
			node.resolve(q);
		}
		// ---------------------- Animation::Channel ----------------------
		Animation::Channel::Channel(const JValue& v):
			sampler(Required<TagSamp>(v, "sampler")),
			target(Required<Target>(v, "target"))
		{}
		void Animation::Channel::resolve(const ITagQuery& q) {
			sampler.resolve(q);
			target.resolve(q);
		}
		bool Animation::Channel::CanLoad(const JValue&) noexcept {
			return true;
		}
		// ---------------------- Animation::Sampler ----------------------
		Animation::Sampler::Sampler(const JValue& v):
			Resource(v),
			input(Required<TagAccessor>(v, "input")),
			output(Required<TagAccessor>(v, "output"))
		{
			if(!v.IsObject())
				throw InvalidProperty("not an object");
			{
				const auto ip = Optional<String>(v, "interpolation", "LINEAR");
				if(std::strcmp(ip, "LINEAR") != 0)
					throw InvalidEnum("");
			}
		}
		void Animation::Sampler::resolve(const ITagQuery& q) {
			input.resolve(q);
			output.resolve(q);
		}
		Resource::Type Animation::Sampler::getType() const noexcept {
			return Type::AnimSampler;
		}
		namespace {
			template <class T>
			using SVec = std::shared_ptr<std::vector<T>>;
		}
		void Animation::Sampler::_checkData() const {
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
		HSampler Animation::Sampler::outputAsTranslation() const {
			_checkData();
			Assert0(!cached.vec4);
			const auto samp = std::make_shared<dc::T_Sampler>();
			samp->value = std::static_pointer_cast<std::vector<frea::Vec3>>(cached.output);
			return samp;
		}
		HSampler Animation::Sampler::outputAsRotation() const {
			_checkData();
			Assert0(cached.vec4);
			const auto samp = std::make_shared<dc::R_Sampler>();
			samp->value = std::static_pointer_cast<std::vector<frea::Quat>>(cached.output);
			return samp;
		}
		HSampler Animation::Sampler::outputAsScaling() const {
			_checkData();
			Assert0(!cached.vec4);
			const auto samp = std::make_shared<dc::S_Sampler>();
			samp->value = std::static_pointer_cast<std::vector<frea::Vec3>>(cached.output);
			return samp;
		}

		// ---------------------- Animation ----------------------
		Animation::Animation(const JValue& v):
			Resource(v),
			sampler(Optional<Dictionary<Shared<Sampler>>>(v, "samplers", {})),
			channel(Optional<Array<Channel>>(v, "channels", {})),
			tagCnv(Optional<Dictionary<StdString>>(v, "parameters", {}))
		{}
		Resource::Type Animation::getType() const noexcept {
			return Type::Animation;
		}
		void Animation::resolve(const ITagQuery& q) {
			struct Q : ITagQuery {
				const Animation& _a;
				const ITagQuery& _next;
				Q(const Animation& a, const ITagQuery& n):
					_a(a),
					_next(n)
				{}
				Void_SP query(Resource::Type type, const Tag& tag) const noexcept override {
					if(type == Type::Accessor) {
						if(const auto itr = _a.tagCnv.find(tag);
							itr != _a.tagCnv.end())
							return _next.query(type, itr->second);
					} else if(type == Type::AnimSampler) {
						if(const auto itr = _a.sampler.find(tag);
							itr != _a.sampler.end())
							return itr->second;
						return nullptr;
					}
					return _next.query(type, tag);
				}
			} qn(*this, q);
			for(auto& s : sampler)
				s.second->resolve(qn);
			for(auto& c : channel)
				c.resolve(qn);
		}
	}
}
#include "gltf/accessor.hpp"
#include "dc/pos_sampler.hpp"
#include "dc/channel.hpp"
#include "dc/joint_at.hpp"
#include "gltf/node.hpp"
namespace rev::gltf {
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
