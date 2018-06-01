#include "gltf/v1/bufferview.hpp"
#include "../check.hpp"
#include "gltf/v1/buffer.hpp"
#include "../../gl_resource.hpp"
#include "../../gl_buffer.hpp"

namespace rev::gltf::v1 {
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
	BufferView::BufferView(const JValue& v, const IDataQuery& q):
		Resource(v),
		src(Required<DRef_Buffer>(v, "buffer", q)),
		byteOffset(Required<Integer>(v, "byteOffset")),
		byteLength(OptionalDefault<Integer>(v, "byteLength", 0))
	{
		auto t = Optional<Integer>(v, "target");
		if(t)
			target = CheckEnum(c_type, *t).type;
	}
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
	const HVb& BufferView::getAsVb() const {
		if(!vb_cached) {
			const auto buff = getBuffer();
			const auto vb = vb_cached = mgr_gl.makeVBuffer(DrawType::Static);
			vb->initData(buff.asPointer(), buff.length);
		}
		return vb_cached;
	}
}
