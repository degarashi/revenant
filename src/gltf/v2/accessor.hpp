#pragma once
#include "gltf/accessor.hpp"
#include "gltf/v2/dataref.hpp"

namespace rev::gltf::v2 {
	class Accessor :
		public gltf::Accessor,
		public Resource
	{
		private:
			struct Sparse {
				struct Values {
					DRef_BufferView		bufferView;
					Size				byteOffset;		// Default: 0

					Values(const JValue& v, const IDataQuery& q);
				};
				struct Indices : Values {
					// UNSIGNED_BYTE or UNSIGNED_SHORT or UNSIGNED_INT
					GLTypeFmt			componentType;

					Indices(const JValue& v, const IDataQuery& q);
				};
				Size			count;
				Indices			indices;
				Values			values;

				Sparse(const JValue& v, const IDataQuery& q);
			};
			using Sparse_OP = spi::Optional<Sparse>;

			using ByteV_OP = spi::Optional<ByteV>;
			mutable ByteV_OP	_cached;
			mutable HVb			_cached_vb;

			Accessor(Temporary_t, const Accessor& self);
		protected:
			Size_OP _getByteStride() const noexcept override;
			DataP _getBufferData() const override;
			void _onCacheMaked(Cache& c) const override;
			bool _filterEnabled() const noexcept override;
		public:
			DRef_BufferView_OP	bufferView;
			bool				normalized;
			Sparse_OP			sparse;

			Accessor(const JValue& v, const IDataQuery& q);
			Type getType() const noexcept override;
			Vb_P getAsVb() const override;
			bool isNormalized() const noexcept override;
	};
}
