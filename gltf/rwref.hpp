#pragma once
#include "../handle/sdl.hpp"
#include "../handle/uri.hpp"
#include "json_types.hpp"
#include "spine/optional.hpp"

namespace rev::gltf {
	struct IDataQueryBase;
	using ByteV = std::vector<uint8_t>;
	using ByteV_OP = spi::Optional<ByteV>;
	class RWRef {
		private:
			HURI				_uri;
			mutable ByteV_OP	_buffer_cached;
		public:
			RWRef(const JValue& v, const IDataQueryBase& q);
			HRW getRW() const;
			const HURI& getUri() const noexcept;

			const ByteV& getBuffer() const;
	};
	using RWRef_OP = spi::Optional<RWRef>;
}
