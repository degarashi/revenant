#include "gltf/bufferview.hpp"
#include "gltf/check.hpp"
#include "gltf/buffer.hpp"

namespace rev::gltf {
	namespace {
		struct BType {
			GLenum		num;
			BufferType	type;
			bool operator == (const GLenum e) const noexcept {
				return num == e;
			}
		};
		const BType c_type[] = {
			{GL_ARRAY_BUFFER, BufferType::Vertex},
			{GL_ELEMENT_ARRAY_BUFFER, BufferType::Index},
		};
	}
	using namespace loader;
	BufferView::BufferView(const JValue& v):
		Resource(v),
		src(Required<String>(v, "buffer")),
		byteOffset(Required<Integer>(v, "byteOffset")),
		byteLength(Optional<Integer>(v, "byteLength", 0))
	{
		auto t = Optional<Integer>(v, "target");
		if(t)
			target = CheckEnum(c_type, *t).type;
	}
	Resource::Type BufferView::getType() const noexcept {
		return Type::BufferView;
	}
	void BufferView::resolve(const ITagQuery& q) {
		src.resolve(q);
	}
	std::pair<uintptr_t, std::size_t> BufferView::getBuffer() const {
		const auto& buff = src.data()->src.getBuffer();
		return {
			reinterpret_cast<uintptr_t>(buff.data()) + byteOffset,
			byteLength
		};
	}
}
