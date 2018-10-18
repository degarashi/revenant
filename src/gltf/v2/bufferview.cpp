#include "gltf/v2/bufferview.hpp"
#include "gltf/value_loader.hpp"
#include "gltf/check.hpp"
#include "gltf/v2/buffer.hpp"

namespace rev::gltf::v2 {
	namespace {
		const std::pair<GLenum, BufferType> c_type[] = {
			{GL_ARRAY_BUFFER, BufferType::Vertex},
			{GL_ELEMENT_ARRAY_BUFFER, BufferType::Index},
		};
	}

	namespace L = ::rev::gltf::loader;
	BufferView::BufferView(const JValue& v, const IDataQuery& q):
		gltf::BufferView(v),
		Resource(v),
		buffer(L::Required<DRef_Buffer>(v, "buffer", q)),
		byteStride(L::Optional<L::Integer>(v, "byteStride"))
	{}
	Resource::Type BufferView::getType() const noexcept {
		return Type::BufferView;
	}
	DataP BufferView::getBuffer() const {
		const auto buff = buffer->getBuffer();
		return {
			.pointer = reinterpret_cast<uintptr_t>(buff.first) + byteOffset,
			.length = byteLength
		};
	}
}
