#include "gltf/image.hpp"

namespace rev::gltf {
	using namespace loader;
	Image::Image(const JValue& v):
		Resource(v),
		src(Required<TagRW>(v, "uri"))
	{}
	Resource::Type Image::getType() const noexcept {
		return Type::Image;
	}
	void Image::resolve(const ITagQuery& q) {
		src.resolve(q);
	}
}
