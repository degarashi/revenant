#include "gltf/v2/texture.hpp"
#include "gltf/v2/image.hpp"
#include "gltf/v2/sampler.hpp"
#include "gltf/v2/bufferview.hpp"
#include "../value_loader.hpp"
#include "../../gl_resource.hpp"
#include "../../gl_texture.hpp"
#include "../../texturesrc_mem.hpp"

namespace rev::gltf::v2 {
	namespace L = gltf::loader;
	Texture::Texture(const JValue& v, const IDataQuery& q):
		Resource(v),
		sampler(L::Optional<DRef_Sampler>(v, "sampler", q)),
		source(L::Optional<DRef_Image>(v, "source", q))
	{}
	Resource::Type Texture::getType() const noexcept {
		return Type::Texture;
	}
	Sampler Texture::_getSampler() const {
		if(sampler)
			return *(*sampler);
		return Sampler::DefaultSampler();
	}
	const HTex& Texture::getGLResource() const {
		if(!tex_cached) {
			if(source) {
				auto samp = _getSampler();
				auto& src = *(*source);
				const auto filter = std::make_shared<TextureFilter>();
				if(src.src) {
					// URIからの読み込み
					const auto uri = src.src->getUri();
					tex_cached = mgr_gl.attachTexFilter(
						mgr_gl.loadTexture(*uri, samp.mipLevel),
						filter
					);
				} else if(src.bufferView && src.mimeType) {
					// BufferViewからの読み込み
					if(*src.mimeType == "image/jpeg" ||
						*src.mimeType ==  "image/png") {
						tex_cached = mgr_gl.attachTexFilter(
							mgr_gl.loadTextureFromRW((*src.bufferView)->getAsRW()),
							filter
						);
					} else
						throw InvalidEnum("unknown mime type");
				} else
					throw LackOfPrerequisite("no source");
				// サンプラーの設定
				filter->setFilter(samp.iLinearMag, samp.iLinearMin);
				filter->setUVWrap(samp.wrapS, samp.wrapT);
			}
		}
		return tex_cached;
	}
}
