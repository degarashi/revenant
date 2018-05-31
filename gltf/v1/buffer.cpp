#include "gltf/v1/buffer.hpp"
#include "gltf/v1/dataquery_if.hpp"
#include "../check.hpp"
#include "../value_loader.hpp"

namespace rev::gltf::v1 {
	namespace {
		const std::string c_type[] = {
			"arraybuffer",
			"text"
		};
	}
	using namespace loader;
	Buffer::Buffer(const JValue& v, const IDataQuery& q):
		Resource(v),
		src(loader::GetRequiredEntry(v, "uri"), q),
		byteLength(OptionalDefault<Integer>(v, "byteLength", 0)),
		type(OptionalDefault<String>(v, "type", "arraybuffer"))
	{
		CheckEnum(c_type, type);
	}
	Buffer::Type Buffer::getType() const noexcept {
		return Resource::Type::Buffer;
	}
}
