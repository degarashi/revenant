#pragma once
#include "resource.hpp"
#include "gltf/v1/dataref.hpp"
#include "gltf/bufferview.hpp"

namespace rev::gltf::v1 {
	struct BufferView :
		gltf::BufferView,
		Resource
	{
		DRef_Buffer		src;

		BufferView(const JValue& v, const IDataQuery& q);
		Type getType() const noexcept override;

		DataP getBuffer() const override;
	};
}
