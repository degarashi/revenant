#include "gltf/v1/accessor.hpp"
#include "../check.hpp"

namespace rev::gltf::v1 {
	namespace L = gltf::loader;
	Accessor::Accessor(const JValue& v, const IDataQuery& q):
		gltf::Accessor(v),
		Resource(v),
		bufferView(L::Required<DRef_BufferView>(v, "bufferView", q))
	{
		if(const auto str = L::Optional<L::Integer>(v, "byteStride")) {
			CheckRange<std::size_t>(*str, 0, 255);
			if(*str > 0)
				byteStride = *str;
		}
		if(byteStride == 0)
			byteStride = GLFormat::QuerySize(_componentType) * _nElem;
	}
	Accessor::Size_OP Accessor::_getByteStride() const noexcept {
		return byteStride;
	}
	DataP Accessor::_getBufferData() const {
		return bufferView->getBuffer();
	}
	bool Accessor::_filterEnabled() const noexcept {
		auto& t = bufferView->target;
		if(t && *t == BufferType::Index)
			return false;
		return true;
	}
	Resource::Type Accessor::getType() const noexcept {
		return Type::Accessor;
	}
	Accessor::Vb_P Accessor::getAsVb() const {
		return {
			.vb = bufferView->getAsVb(),
			.offset = _byteOffset
		};
	}
	bool Accessor::isNormalized() const noexcept {
		return false;
	}
}
