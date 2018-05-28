#include "gltf/v1/sampler.hpp"
#include "../value_loader.hpp"
#include "../check.hpp"

namespace rev::gltf::v1 {
	namespace {
		struct Filter {
			GLenum		type;
			int			iLinear;
			MipState	mip;

			bool operator == (const GLenum t) const noexcept {
				return type == t;
			}
		};
		const Filter c_mag[] = {
			{GL_NEAREST,					0,		MipState::NoMipmap},
			{GL_LINEAR,						1,		MipState::NoMipmap},
		};
		const Filter c_min[] = {
			{GL_NEAREST,					0,		MipState::NoMipmap},
			{GL_LINEAR,						0,		MipState::NoMipmap},
			{GL_NEAREST_MIPMAP_NEAREST,		0,		MipState::MipmapNear},
			{GL_LINEAR_MIPMAP_NEAREST,		0,		MipState::MipmapLinear},
			{GL_NEAREST_MIPMAP_LINEAR,		1,		MipState::MipmapNear},
			{GL_LINEAR_MIPMAP_LINEAR,		1,		MipState::MipmapLinear},
		};
		struct Wrap {
			GLenum		type;
			WrapState	state;

			bool operator == (const GLenum t) const noexcept {
				return type == t;
			}
		};
		const Wrap c_wrap[] = {
			{GL_CLAMP_TO_EDGE,		WrapState::ClampToEdge},
			{GL_MIRRORED_REPEAT,	WrapState::MirroredRepeat},
			{GL_REPEAT,				WrapState::Repeat},
		};
	}
	using namespace loader;
	Sampler::Sampler(const JValue& v):
		Resource(v)
	{
		iLinearMag = CheckEnum(c_mag, OptionalDefault<loader::GLEnum>(v, "magFilter", GL_LINEAR)).iLinear;
		const auto& min = CheckEnum(c_min, OptionalDefault<loader::GLEnum>(v, "minFilter", GL_NEAREST_MIPMAP_LINEAR));
		iLinearMin = min.iLinear;
		mipLevel = min.mip;

		wrapS = CheckEnum(c_wrap, OptionalDefault<loader::GLEnum>(v, "wrapS", GL_REPEAT)).state;
		wrapT = CheckEnum(c_wrap, OptionalDefault<loader::GLEnum>(v, "wrapT", GL_REPEAT)).state;
	}
	Resource::Type Sampler::getType() const noexcept {
		return Type::Sampler;
	}
}
