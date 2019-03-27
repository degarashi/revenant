#include "gltf/v2/accessor.hpp"
#include "gltf/v2/bufferview.hpp"
#include "../check.hpp"
#include "../value_loader.hpp"
#include "../../gl/resource.hpp"
#include "../../gl/buffer.hpp"

namespace rev::gltf::v2 {
	namespace L = gltf::loader;
	// ------------------- Accessor::Sparse::Values -------------------
	Accessor::Sparse::Values::Values(const JValue& v, const IDataQuery& q):
		bufferView(L::Required<DRef_BufferView>(v, "bufferView", q)),
		byteOffset(L::OptionalDefault<L::Integer>(v, "byteOffset", 0))
	{}
	// ------------------- Accessor::Sparse::Indices -------------------
	Accessor::Sparse::Indices::Indices(const JValue& v, const IDataQuery& q):
		Values(v, q),
		componentType(L::Required<L::Integer>(v, "componentType"))
	{}
	// ------------------- Accessor::Sparse -------------------
	Accessor::Sparse::Sparse(const JValue& v, const IDataQuery& q):
	count(L::Required<L::Integer>(v, "count")),
	indices(L::Required<Indices>(v, "indices", q)),
		values(L::Required<Values>(v, "values", q))
	{}
	// ------------------- Accessor -------------------
	Accessor::Accessor(
		Temporary_t,
		const Accessor& self
	):
		gltf::Accessor(Temporary, self),
		Resource(nullptr),
		normalized(false)
	{}
	Accessor::Accessor(const JValue& v, const IDataQuery& q):
		gltf::Accessor(v),
		Resource(v),
		bufferView(L::Optional<DRef_BufferView>(v, "bufferView", q)),
		normalized(L::OptionalDefault<L::Bool>(v, "normalized", false)),
		sparse(L::Optional<Sparse>(v, "sparse", q))
	{}
	Accessor::Size_OP Accessor::_getByteStride() const noexcept {
		if(bufferView)
			return (*bufferView)->byteStride;
		return spi::none;
	}
	DataP Accessor::_getBufferData() const {
		if(!_cached) {
			ByteV cached;
			if(bufferView) {
				const auto data = (*bufferView)->getBuffer();
				const auto* ptr = reinterpret_cast<const uint8_t*>(data.pointer);
				cached.assign(
					ptr,
					ptr + data.length
				);
			} else {
				const auto unitsize = *GLFormat::QuerySize(_componentType);
				cached.resize(unitsize*_nElem*_count);
			}
			// Sparse反映
			if(sparse) {
				auto& sp = *sparse;

				auto& val = sp.values;
				Accessor ac_v(Temporary, *this);
				ac_v.bufferView = val.bufferView;
				ac_v._byteOffset = val.byteOffset;
				ac_v._count = sp.count;
				const auto src = ac_v.getDataP_Unit();

				auto& idx = sp.indices;
				Accessor ac_i(Temporary, *this);
				ac_i.bufferView = idx.bufferView;
				ac_i._byteOffset = idx.byteOffset;
				ac_i._count = sp.count;
				ac_i._componentType = idx.componentType;
				ac_i._nElem = 1;

				const auto idxv = ac_i.cnvToInt32();
				std::size_t v_idx = 0;
				for(auto& i : idxv) {
					std::memcpy(
						cached.data()+src.unitSize*i,
						reinterpret_cast<const void*>(src.pointer + src.unitSize*v_idx),
						src.unitSize
					);
					++v_idx;
				}
			}
			_cached = std::move(cached);
		}
		return {
			.pointer = reinterpret_cast<uintptr_t>(_cached->data()),
			.length = _cached->size()
		};
	}
	bool Accessor::_filterEnabled() const noexcept {
		if(bufferView) {
			auto& t = (*bufferView)->target;
			if(t && *t == BufferType::Index)
				return false;
		}
		return true;
	}
	Resource::Type Accessor::getType() const noexcept {
		return Type::Accessor;
	}
	namespace {
		struct Normalize {
			GLfloat operator()(const GLbyte v) noexcept {
				return std::max(v / 127.f, -1.f); }
			GLfloat operator()(const GLubyte v) noexcept {
				return v / 255.f; }
			GLfloat operator()(const GLshort v) noexcept {
				return std::max(v / 32767.f, -1.f); }
			GLfloat operator()(const GLushort v) noexcept {
				return v / 65535.f; }
			template <class T>
			T operator()(const T t) noexcept {
				return t; }
		};
	}
	void Accessor::_onCacheMaked(Cache& cache) const {
		if(normalized) {
			const auto len = getActualNElem() * _count;
			Cache nc(sizeof(float) * len);
			_SelectByType(_componentType, [len, &cache](auto typ){
				using Type = decltype(typ);
				auto* dst = reinterpret_cast<Type*>(cache.data());
				Normalize nml;
				for(std::size_t i=0 ; i<len ; i++) {
					*dst = nml(*dst);
					++dst;
				}
			});
			cache = std::move(nc);
		}
	}
	Accessor::Vb_P Accessor::getAsVb() const {
		if(!sparse) {
			if(bufferView) {
				return {
					.vb = (*bufferView)->getAsVb(),
					.offset = _byteOffset
				};
			}
		}
		if(!_cached_vb) {
			const auto data = _getBufferData();
			const auto vb = _cached_vb = mgr_gl.makeVBuffer(DrawType::Static);
			vb->initData(data.asPointer(), data.length);
		}
		return {
			.vb = _cached_vb,
			.offset = 0
		};
	}
	bool Accessor::isNormalized() const noexcept {
		return normalized;
	}
}
