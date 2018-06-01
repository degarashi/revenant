#include "gltf/v1/bufferview.hpp"
#include "gltf/v1/buffer.hpp"
#include "../check.hpp"

namespace rev::gltf::v1 {
	namespace L = ::rev::gltf::loader;
	BufferView::BufferView(const JValue& v, const IDataQuery& q):
		gltf::BufferView(v),
		Resource(v),
		src(L::Required<DRef_Buffer>(v, "buffer", q))
	{}
	Resource::Type BufferView::getType() const noexcept {
		return Type::BufferView;
	}
	DataP BufferView::getBuffer() const {
		const auto& buff = src->src.getBuffer();
		return {
			.pointer = reinterpret_cast<uintptr_t>(buff.data()) + byteOffset,
			.length = byteLength
		};
	}
}
