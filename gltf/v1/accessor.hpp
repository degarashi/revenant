#pragma once
#include "gltf/v1/resource.hpp"
#include "gltf/v1/bufferview.hpp"
#include "gltf/accessor.hpp"

namespace rev::gltf::v1 {
	//! BufferViewが指すバイナリデータを型情報付きで参照
	struct Accessor :
		gltf::Accessor,
		Resource
	{
		// BufferViewに対するオフセット、間隔
		Size				byteStride;
		// 参照元のバイナリデータ
		DRef_BufferView		bufferView;

		Size_OP _getByteStride() const noexcept override;
		DataP _getBufferData() const override;
		bool _filterEnabled() const noexcept override;

		Accessor(const JValue& v, const IDataQuery& q);
		Type getType() const noexcept override;
		Vb_P getAsVb() const override;
	};
}
