#include "gltf/bufferview.hpp"
#include "gltf/value_loader.hpp"
#include "gltf/check.hpp"
#include "../gl_resource.hpp"
#include "../gl_buffer.hpp"
#include "../sdl_rw.hpp"

namespace rev::gltf {
	namespace {
		const std::pair<GLenum, BufferType> c_type[] = {
			{GL_ARRAY_BUFFER, BufferType::Vertex},
			{GL_ELEMENT_ARRAY_BUFFER, BufferType::Index},
		};
	}
	namespace L = ::rev::gltf::loader;
	BufferView::BufferView(const JValue& v):
		byteOffset(L::OptionalDefault<L::Integer>(v, "byteOffset", 0)),
		byteLength(L::Required<L::Integer>(v, "byteLength"))
	{
		if(const auto t = L::Optional<L::Integer>(v, "target")) {
			target = CheckEnum(c_type, *t, [](auto& c, auto& v){
						return c.first == v;
					}).second;
		}
	}
	const HVb& BufferView::getAsVb() const {
		if(!vb_cached) {
			const auto buff = getBuffer();
			const auto vb = vb_cached = mgr_gl.makeVBuffer(DrawType::Static);
			vb->initData(buff.asPointer(), buff.length);
		}
		return vb_cached;
	}
	HRW BufferView::getAsRW() const {
		const auto buff = getBuffer();
		return mgr_rw.fromConstTemporal(buff.asPointer(), buff.length);
	}
}
