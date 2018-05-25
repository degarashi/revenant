#include "gltf/texture.hpp"
#include "gltf/value_loader.hpp"
#include "gltf/check.hpp"
#include "../gl_resource.hpp"
#include "gltf/image.hpp"
#include "gltf/sampler.hpp"
#include "../gl_texture.hpp"

namespace rev::gltf {
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
	HTex Texture::getGLResource() const {
		auto& samp = *sampler.data();
		// URIからテクスチャ生成
		const auto uri = source.data()->src.getUri();
		HTex tex = mgr_gl.loadTexture(*uri, samp.mipLevel);
		// サンプラーの設定
		tex->setFilter(samp.iLinearMag, samp.iLinearMin);
		tex->setUVWrap(samp.wrapS, samp.wrapT);
		return tex;
	}
}
