#include "gltf/buffer.hpp"
#include "gltf/check.hpp"
#include "../gl_resource.hpp"
#include "../gl_buffer.hpp"

namespace rev::gltf {
	namespace {
		const std::string c_type[] = {
			"arraybuffer",
			"text"
		};
	}
	using namespace loader;
	Buffer::Buffer(const JValue& v):
		Resource(v),
		src(loader::GetRequiredEntry(v, "uri")),
		byteLength(Optional<Integer>(v, "byteLength", 0)),
		type(Optional<String>(v, "type", "arraybuffer"))
	{
		CheckEnum(c_type, type);
	}
	Buffer::Type Buffer::getType() const noexcept {
		return Resource::Type::Buffer;
	}
	void Buffer::resolve(const ITagQuery& q) {
		src.resolve(q);
	}

	const HVb& Buffer::getAsVb() const {
		if(!vb_cached) {
			const auto& buff = src.getBuffer();
			const auto vb = vb_cached = mgr_gl.makeVBuffer(DrawType::Static);
			vb->initData(buff, 0);
		}
		return vb_cached;
	}
}
