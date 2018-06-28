#include "gltf/accessor.hpp"
#include "gltf/value_loader.hpp"
#include "gltf/check.hpp"
#include "lubee/meta/types.hpp"

namespace rev::gltf {
	namespace {
		using A = Accessor;
	}
	template <class V>
	struct A::Filter : A::IFilter {
		const V		_min,
					_max;

		Filter(const V min, const V max):
			_min(min),
			_max(max)
		{}
		void filter(const uintptr_t data) noexcept override {
			auto& d = *reinterpret_cast<V*>(data);
			d = std::min(std::max(d, _min), _max);
		}
	};
	template <class V>
	struct A::FilterG : A::IFilter {
		using FV = std::vector<Filter<V>>;
		FV			_filter;
		std::size_t	_cursor;

		FilterG(FV&& f):
			_filter(std::move(f)),
			_cursor(0)
		{}
		void filter(const uintptr_t data) noexcept override {
			_filter[_cursor].filter(data);
			_cursor = (_cursor+1) % _filter.size();
		}
	};

	namespace {
		template <class To, class From>
		std::size_t Cnv(uintptr_t dst, uintptr_t src) noexcept {
			auto* srcP = reinterpret_cast<const From*>(src);
			auto* dstP = reinterpret_cast<To*>(dst);
			*dstP = *srcP;
			return sizeof(To);
		}
		const GLenum c_componentType[] = {
			GL_BYTE,
			GL_UNSIGNED_BYTE,
			GL_SHORT,
			GL_UNSIGNED_SHORT,
			GL_UNSIGNED_INT,
			GL_FLOAT
		};
		struct ElemType {
			const std::string	name;
			bool				bMat;
			std::size_t			nElem;

			bool operator == (const char* c) const noexcept {
				return name == c;
			}
		};
		const ElemType c_elemtype[] = {
			{"SCALAR", false, 1},
			{"VEC2", false, 2},
			{"VEC3", false, 3},
			{"VEC4", false, 4},
			{"MAT2", true, 2},
			{"MAT3", true, 3},
			{"MAT4", true, 4}
		};
	}
	namespace L = gltf::loader;
	A::Accessor(
		Temporary_t,
		const A& self
	):
		_componentType(self._componentType),
		_byteOffset(self._byteOffset),
		_count(self._count),
		_nElem(self._nElem),
		_bMatrix(self._bMatrix),
		_bFloat(self._bFloat)
	{}
	A::Accessor(const JValue& v):
		_componentType(L::Required<L::Integer>(v, "componentType")),
		_byteOffset(L::OptionalDefault<L::Integer>(v, "byteOffset", 0)),
		_count(L::Required<L::Integer>(v, "count"))
	{
		CheckRange<Size>(_byteOffset, 0);
		CheckEnum(c_componentType, _componentType);
		CheckRange<Size>(_count, 1);
		_bFloat = _componentType == GL_FLOAT;

		const auto& typ = CheckEnum(c_elemtype, L::Required<L::String>(v, "type"));
		_bMatrix = typ.bMat;
		_nElem = typ.nElem;

		if(const auto& max = L::GetOptionalEntry(v, "max")) {
			const auto& min = L::GetRequiredEntry(v, "min");
			const auto n = max->Size();
			if(n != min.Size() ||
				getActualNElem() != n)
				throw InvalidProperty("size(max) and size(min) differs");

			const auto maxV = L::Required<L::Array<L::Number>>(v, "max");
			const auto minV = L::Required<L::Array<L::Number>>(v, "min");
			D_Assert0(maxV.size() == minV.size());

			if(minV.size() == 3) {
				_bbox = dc::BBox{
					{minV[0], minV[1], minV[2]},
					{maxV[0], maxV[1], maxV[2]}
				};
			}

			Filter_S filter;
			_SelectByType(_componentType, [&filter, &minV, &maxV](auto type){
				using Type = decltype(type);
				std::vector<Filter<Type>> fl;
				const auto len = minV.size();
				for(std::size_t i=0 ; i<len ; i++) {
					fl.emplace_back(minV[i], maxV[i]);
				}
				filter = std::make_shared<FilterG<Type>>(std::move(fl));
			});
			_filter = filter;
		}
		D_Assert0(lubee::IsInRange<Size>(_nElem, 1, 4));
	}
	std::size_t A::getActualNElem() const {
		if(_bMatrix)
			return _nElem*_nElem;
		return _nElem;
	}
	void A::_MatrixTranspose(
		void* data,
		const Size unit,
		const Size dim,
		const Size n
	) {
		if(unit == 1)
			_MatrixTranspose0<uint8_t>(data, dim, n);
		else if(unit == 2)
			_MatrixTranspose0<uint16_t>(data, dim, n);
		else if(unit == 4)
			_MatrixTranspose0<uint32_t>(data, dim, n);
		else
			D_Assert0(false);
	}
	const A::Cache& A::_getCache() const {
		if(_cache.empty()) {
			auto data = _getBufferData();
			data.pointer += _byteOffset;
			const Size stride = getByteStride();
			const auto count = _count,
					nElem = _nElem;

			Size unit;
			_SelectByType(_componentType, [&unit](auto type){
				using Type = decltype(type);
				unit = sizeof(Type);
			});

			_cache.resize(unit * getActualNElem() * count);
			auto dst = reinterpret_cast<uintptr_t>(_cache.data());
			if(_bMatrix) {
				D_Assert0(_bFloat);
				for(Size i=0 ; i<count ; i++) {
					auto src = data.pointer;
					for(Size j=0 ; j<nElem ; j++) {
						src = (src+3) / 4 * 4;
						for(Size k=0 ; k<nElem ; k++) {
							dst += Cnv<float, float>(dst, src);
							src += sizeof(float);
						}
					}
					data.pointer += stride;
				}
				_applyFilter();
				_MatrixTranspose(_cache.data(), sizeof(float), nElem, count);
			} else {
				if(nElem == 1) {
					_SelectByType(_componentType,
						[
							&dst,
							count,
							nElem,
							src=data.pointer,
							stride
						](auto type) mutable {
							using Type = decltype(type);
							for(Size i=0 ; i<count*nElem ; i++) {
								dst += Cnv<Type, Type>(dst, src);
								src += stride;
							}
						}
					);
				} else {
					auto load = [count, data, &dst, nElem, stride](auto dsttype) mutable {
						using DstType = decltype(dsttype);
						for(Size i=0 ; i<count ; i++) {
							auto src = data.pointer;
							for(Size j=0 ; j<nElem ; j++) {
								dst += Cnv<DstType, float>(dst, src);
								src += sizeof(float);
							}
							data.pointer += stride;
						}
					};
					if(_bFloat) {
						load(float{});
					} else {
						load(int32_t{});
					}
				}
				_applyFilter();
			}
			D_Assert0(dst == reinterpret_cast<uintptr_t>(_cache.data() + _cache.size()));
			_onCacheMaked(_cache);
		}
		return _cache;
	}
	void A::_applyFilter() const {
		if(_filter && _filterEnabled()) {
			const auto len = _count * getActualNElem();
			const auto unit = getUnitSize();
			auto data = reinterpret_cast<uintptr_t>(_cache.data());
			for(std::size_t i=0 ; i<len ; i++) {
				_filter->filter(data);
				data += unit;
			}
			D_Assert0(data == reinterpret_cast<uintptr_t>(_cache.data() + _cache.size()));
		}
	}
	namespace {
		namespace visitor {
			template <class To>
			struct Cnv {
				A::Vec<To>		result;

				template <class T, ENABLE_IF(lubee::is_number<T>{})>
				void operator()(const T* t, const std::size_t len) {
					result.resize(len);
					for(std::size_t i=0 ; i<len ; i++) {
						result[i] = t[i];
					}
				}
				template <class V, ENABLE_IF(frea::is_vector<V>{})>
				void operator()(const V* v, const std::size_t len) {
					return (*this)(reinterpret_cast<const typename V::value_t*>(v), len*V::size);
				}
				template <class M, ENABLE_IF(frea::is_matrix<M>{})>
				void operator()(const M* m, const std::size_t len) {
					return (*this)(reinterpret_cast<const typename M::value_t*>(m), len*M::dim_m*M::dim_n);
				}
			};
			struct MakeInfo {
				DataP_Unit result;
				template <class T>
				void operator()(const T* t, const std::size_t len) {
					result = {
						DataP{
							.pointer = reinterpret_cast<uintptr_t>(t),
							.length = len
						},
						.unitSize = sizeof(t[0]),
					};
				}
			};
		}
	}
	const dc::BBox_Op& A::getBBox() const {
		return _bbox;
	}
	A::Vec<int32_t> A::cnvToInt32() const {
		visitor::Cnv<int32_t> cnv;
		getData(cnv);
		return std::move(cnv.result);
	}
	A::Vec<float> A::cnvToFloat() const {
		visitor::Cnv<float> cnv;
		getData(cnv);
		return std::move(cnv.result);
	}
	DataP_Unit A::getDataP_Unit() const {
		visitor::MakeInfo v;
		getData(v);
		return v.result;
	}
	std::size_t A::getUnitSize() const {
		Size ret;
		_SelectByType(_componentType, [&ret](auto raw){
			ret = sizeof(raw);
		});
		return ret;
	}
	A::Size A::getByteStride() const noexcept {
		if(const auto s = _getByteStride())
			return *s;
		return getUnitSize() * getActualNElem();
	}
}
