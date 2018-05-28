#include "gltf/v1/image.hpp"
#include "../value_loader.hpp"

namespace rev::gltf::v1 {
	using namespace loader;
	Image::Image(const JValue& v, const IDataQuery& q):
		Resource(v),
		src(Required<RWRef>(v, "uri", q))
	{}
	Resource::Type Image::getType() const noexcept {
		return Type::Image;
	}
}
