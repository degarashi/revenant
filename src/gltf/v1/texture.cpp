#include "gltf/v1/texture.hpp"
#include "../value_loader.hpp"
#include "../check.hpp"
#include "../../gl/resource.hpp"
#include "gltf/v1/image.hpp"
#include "gltf/v1/sampler.hpp"
#include "../../gl/texture.hpp"

namespace rev::gltf::v1 {
	namespace {
		const GLenum c_target[] = {GL_TEXTURE_2D};
		const GLenum c_type[] = {
			GL_UNSIGNED_BYTE,
			GL_UNSIGNED_SHORT_5_6_5,
			GL_UNSIGNED_SHORT_4_4_4_4,
			GL_UNSIGNED_SHORT_5_5_5_1,
		};
		const GLenum c_format[] = {
			GL_ALPHA,
			GL_RGB,
			GL_RGBA,
			GL_LUMINANCE,
			GL_LUMINANCE_ALPHA,
		};
	}
	using namespace loader;
	Texture::Texture(const JValue& v, const IDataQuery& q):
		Resource(v),
		format(OptionalDefault<loader::GLEnum>(v, "format", GL_RGBA)),
		internalFormat(OptionalDefault<loader::GLEnum>(v, "internalFormat", GL_RGBA)),
		sampler(Required<DRef_Sampler>(v, "sampler", q)),
		source(Required<DRef_Image>(v, "source", q)),
		type(OptionalDefault<loader::GLEnum>(v, "type", GL_UNSIGNED_BYTE))
	{
		CheckEnum(c_format, format);
		CheckEnum(c_format, internalFormat);
		CheckEnum(c_target, OptionalDefault<loader::GLEnum>(v, "target", GL_TEXTURE_2D));
		CheckEnum(c_type, type);
	}
	Resource::Type Texture::getType() const noexcept {
		return Type::Texture;
	}
	const HTex& Texture::getGLResource() const {
		if(!tex_cached) {
			auto& samp = *sampler.data();
			// URIからテクスチャ生成
			const auto uri = source.data()->src.getUri();
			// サンプラーの設定
			auto f = mgr_gl.createTexFilter();
			f->setFilter(samp.iLinearMag, samp.iLinearMin);
			f->setUVWrap(samp.wrapS, samp.wrapT);
			tex_cached = mgr_gl.attachTexFilter(mgr_gl.loadTexture(*uri, samp.mipLevel), f);
		}
		return tex_cached;
	}
}
