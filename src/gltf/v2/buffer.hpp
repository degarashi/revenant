#pragma once
#include "gltf/v2/resource.hpp"
#include "../rwref.hpp"
#include "../../handle/opengl.hpp"

namespace rev::gltf::v2 {
	struct IDataQuery;
	class Buffer :
		public Resource
	{
		private:
			using Data = std::pair<const void*, Size>;

		public:
			RWRef_OP		uri;
			Size			byteLength;
			Data			data;

			Buffer(const JValue& v, const IDataQuery& q);
			Type getType() const noexcept override;

			mutable HVb	cached_vb;
			const Data& getBuffer() const;
			const HVb& getAsVb() const;

			Buffer(Buffer&& b) noexcept;
			Buffer& operator = (Buffer&&) noexcept;
	};
}
