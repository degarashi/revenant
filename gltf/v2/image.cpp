#include "gltf/v2/image.hpp"
#include "gltf/v2/dataquery_if.hpp"
#include "../value_loader.hpp"

namespace rev::gltf::v2 {
	namespace L = gltf::loader;
	Image::Image(const JValue& v, const IDataQuery& q):
		Resource(v),
		src(L::Optional<RWRef>(v, "uri", q)),
		mimeType(L::Optional<L::String>(v, "mimeType")),
		bufferView(L::Optional<DRef_BufferView>(v, "bufferView", q))
	{}
	Resource::Type Image::getType() const noexcept {
		return Type::Image;
	}
}
