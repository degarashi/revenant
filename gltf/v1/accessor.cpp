#include "gltf/v1/accessor.hpp"
#include "../check.hpp"

namespace rev::gltf::v1 {
	namespace L = gltf::loader;
	Accessor::Accessor(const JValue& v, const IDataQuery& q):
		gltf::Accessor(v),
		Resource(v),
		byteStride(L::OptionalDefault<L::Integer>(v, "byteStride", 0)),
		bufferView(L::Required<DRef_BufferView>(v, "bufferView", q))
	{
		CheckRange<std::size_t>(byteStride, 0, 255);
		if(byteStride == 0)
			byteStride = GLFormat::QuerySize(_componentType) * _nElem;
	}
	Accessor::Size_OP Accessor::_getByteStride() const noexcept {
		return byteStride;
	}
	Accessor::Data Accessor::_getBufferData() const {
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
}
