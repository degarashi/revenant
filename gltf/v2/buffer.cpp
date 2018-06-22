#include "gltf/v2/buffer.hpp"
#include "gltf/v2/dataquery_if.hpp"
#include "gltf/value_loader.hpp"
#include "gltf/rwref.hpp"
#include "../../gl_resource.hpp"
#include "../../gl_buffer.hpp"

namespace rev::gltf::v2 {
	namespace L = ::rev::gltf::loader;
	Buffer::Buffer(const JValue& v, const IDataQuery& q):
		Resource(v),
		uri(L::Optional<RWRef>(v, "uri", q)),
		byteLength(L::Required<L::Integer>(v, "byteLength"))
	{
		const auto& buff = uri ? uri->getBuffer() : q.getBinary();
		data = Data{
			buff.data(),
			buff.size()
		};
	}
	Buffer::Buffer(Buffer&& b) noexcept:
		Resource(std::move(static_cast<Resource&>(b))),
		uri(std::move(b.uri)),
		byteLength(b.byteLength),
		cached_vb(std::move(b.cached_vb))
	{
		if(uri) {
			// ポインタを再設定
			const auto& buff = uri->getBuffer();
			data = Data{
				buff.data(),
				buff.size()
			};
		} else {
			// 内部バイナリからのポインタはそのままコピー
			data = b.data;
		}
	}
	Buffer& Buffer::operator = (Buffer&& b) noexcept {
		this->~Buffer();
		new(this) Buffer(std::move(b));
		return *this;
	}

	Buffer::Type Buffer::getType() const noexcept {
		return Resource::Type::Buffer;
	}
	const Buffer::Data& Buffer::getBuffer() const {
		return data;
	}
	const HVb& Buffer::getAsVb() const {
		if(!cached_vb) {
			const auto& buff = uri->getBuffer();
			cached_vb = mgr_gl.makeVBuffer(DrawType::Static);
			cached_vb->initData(buff, 0);
		}
		return cached_vb;
	}
}
