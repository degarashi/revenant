#include "gltf/accessor.hpp"
#include "gltf/value_loader.hpp"
#include "gltf/check.hpp"
#include "lubee/meta/types.hpp"

namespace rev::gltf {
	namespace {
		const GLenum c_componentType[] = {
			GL_BYTE,
			GL_UNSIGNED_BYTE,
			GL_SHORT,
			GL_UNSIGNED_SHORT,
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
			{"MAT2", true, 4},
			{"MAT3", true, 9},
			{"MAT4", true, 16}
		};
	}
	namespace L = gltf::loader;
	Accessor::Accessor(
		Temporary_t,
		const Accessor& self
	):
		_componentType(self._componentType),
		_byteOffset(self._byteOffset),
		_count(self._count),
		_nElem(self._nElem),
		_bMatrix(self._bMatrix),
		_bFloat(self._bFloat)
	{}
	Accessor::Accessor(const JValue& v):
		_componentType(L::Required<L::Integer>(v, "componentType")),
		_byteOffset(L::Required<L::Integer>(v, "byteOffset")),
		_count(L::Required<L::Integer>(v, "count"))
	{
		CheckRange<Size>(_byteOffset, 0);
		CheckEnum(c_componentType, _componentType);
		CheckRange<Size>(_count, 1);
		_bFloat = _componentType == GL_FLOAT;

		const auto& typ = CheckEnum(c_elemtype, L::Required<L::String>(v, "type"));
		if(const auto& max = L::GetOptionalEntry(v, "max")) {
			const auto& min = L::GetRequiredEntry(v, "min");
			const auto n = max->Size();
			if(n != min.Size() ||
				typ.nElem != n)
				throw InvalidProperty("size(max) and size(min) differs");

			const auto maxV = L::Required<L::Array<L::Number>>(v, "max");
			const auto minV = L::Required<L::Array<L::Number>>(v, "min");
			D_Assert0(maxV.size() == minV.size());

			_filter.resize(n);
			for(Size i=0 ; i<n ; i++)
				_filter[i] = {minV[i], maxV[i]};
		}
		_bMatrix = typ.bMat;
		_nElem = typ.nElem;
		D_Assert0(lubee::IsInRange<Size>(_nElem, 1, 16));
	}

	namespace {
		template <class CB>
		void SelectByType(const GLTypeFmt type, CB&& cb) {
			switch(type) {
				case GL_BYTE:
					cb(GLubyte{});
					break;
				case GL_UNSIGNED_BYTE:
					cb(GLubyte{});
					break;
				case GL_SHORT:
					cb(GLshort{});
					break;
				case GL_UNSIGNED_SHORT:
					cb(GLushort{});
					break;
				case GL_FLOAT:
					cb(GLfloat{});
					break;
				default:
					Assert0(false);
			}
		}
		// イテレータから値を成形して出力
		namespace cnv {
			struct Scalar {
				constexpr static std::size_t NElem = 1;

				template <class Itr>
				auto operator ()(Itr itr) const noexcept {
					return *itr;
				}
			};
			// イテレータからベクトル値<Dim>を出力
			template <std::size_t Dim>
			struct Vec {
				constexpr static std::size_t NElem = Dim;

				template <class Itr>
				auto operator ()(Itr itr) const noexcept {
					using value_t = frea::Vec_t<std::decay_t<decltype(*itr)>, Dim, false>;
					value_t ret;
					for(std::size_t i=0 ; i<Dim ; i++) {
						ret.m[i] = *itr;
						++itr;
					}
					return ret;
				}
			};
			template <std::size_t DimM, std::size_t DimN>
			struct Mat {
				constexpr static std::size_t NElem = DimM*DimN;

				template <class Itr>
				auto operator ()(Itr itr) const noexcept {
					using value_t = frea::Mat_t<std::decay_t<decltype(*itr)>, DimM, DimN, false>;
					value_t ret;
					for(std::size_t i=0 ; i<DimM ; i++) {
						auto& vec = ret.m[i];
						for(std::size_t j=0 ; j<DimN ; j++) {
							vec.m[j] = *itr;
							++itr;
						}
					}
					// column-major -> row-majorへの変換
					ret.transpose();
					return ret;
				}
			};
		}
		template <class Src_Unit, class Dst_Unit>
		struct ItrSingle {
			uintptr_t		cur;
			std::size_t		stride;

			ItrSingle(const uintptr_t t, const std::size_t stride):
				cur(t),
				stride(stride)
			{}
			ItrSingle& operator ++ () noexcept {
				cur += stride;
				return *this;
			}
			ItrSingle& operator += (const std::size_t n) noexcept {
				cur += stride*n;
				return *this;
			}
			ItrSingle operator + (const std::size_t n) const noexcept {
				return {cur+stride*n, stride};
			}
			intptr_t operator - (const ItrSingle& itr) const noexcept {
				const intptr_t diff = itr.cur - cur;
				D_Assert0(diff % stride == 0);
				return diff / stride;
			}
			Dst_Unit operator * () const noexcept {
				return *reinterpret_cast<Src_Unit*>(cur);
			}
			bool operator == (const ItrSingle& itr) const noexcept {
				return cur == itr.cur;
			}
			bool operator != (const ItrSingle& itr) const noexcept {
				return !(this->operator ==(itr));
			}
		};
		template <class Cnv, class ItrS>
		struct ComposeIterator {
			Cnv			cnv;
			ItrS		itr;
			std::size_t	stride;

			ComposeIterator(const Cnv cnv, const ItrS itr):
				cnv(cnv),
				itr(itr),
				stride(Cnv::NElem)
			{}
			auto operator * () const noexcept {
				return cnv(itr);
			}
			ComposeIterator& operator ++ () noexcept {
				itr += stride;
				return *this;
			}
			ComposeIterator operator + (const std::size_t n) const noexcept {
				return {cnv, itr+n*stride};
			}
			intptr_t operator - (const ComposeIterator& i) const noexcept {
				const intptr_t diff = i.itr - itr;
				D_Assert0(diff % stride == 0);
				return diff / stride;
			}
			bool operator == (const ComposeIterator& i) const noexcept {
				return itr == i.itr;
			}
			bool operator != (const ComposeIterator& i) const noexcept {
				return !(this->operator ==(i));
			}
		};
	}
	namespace {
		namespace visitor {
			struct Filter {
				using FilterP = std::pair<double,double>;
				using Size = std::size_t;
				const FilterP* filter;

				Filter(const FilterP* filter):
					filter(filter)
				{}
				void operator()(boost::blank) const {
					Assert0(false);
				}
				template <class T, ENABLE_IF(lubee::is_number<T>{})>
				void operator()(std::vector<T>& data) const {
					const auto f = filter[0];
					for(auto& d : data) {
						d = lubee::Saturate<double>(d, f.first, f.second);
					}
				}
				template <class T, ENABLE_IF(frea::is_vector<T>{})>
				void operator()(std::vector<T>& data) const {
					for(auto& d : data) {
						for(Size i=0 ; i<T::size ; i++) {
							auto& f = filter[i];
							d[i] = lubee::Saturate<double>(d[i], f.first, f.second);
						}
					}
				}
				template <class T, ENABLE_IF(frea::is_matrix<T>{})>
				void operator()(std::vector<T>& data) const {
					for(auto& d : data) {
						for(Size i=0 ; i<T::dim_m ; i++) {
							auto& row = d[i];
							for(Size j=0 ; j<T::dim_n ; j++) {
								auto& f = filter[j*T::dim_n + i];
								row[j] = lubee::Saturate<double>(row[j], f.first, f.second);
							}
						}
					}
				}
			};
		}
	}
	template <class BuffType, class Cnv>
	void Accessor::_readCache() const {
		SelectByType(_componentType, [this](auto type){
			auto data = _getBufferData();
			data.pointer += _byteOffset;

			Size stride;
			if(const auto s = _getByteStride())
				stride = *s;
			else
				stride = sizeof(BuffType) * _nElem;

			using SrcType = decltype(type);
			using ItrS = ItrSingle<SrcType, BuffType>;
			ComposeIterator itr{Cnv{}, ItrS{data.pointer, sizeof(SrcType)}};

			using SaveType = std::decay_t<decltype(*itr)>;
			Vec<SaveType> result(_count);
			auto* dst = result.data();
			const auto itrE = itr + _count;
			while(itr != itrE) {
				*dst++ = *itr;
				++itr;
			}
			D_Assert0(dst == result.data()+result.size());
			_cache = std::move(result);
		});
	}
	const Accessor::Cache& Accessor::getData() const {
		if(_cache.which() == 0) {
			if(_bMatrix) {
				switch(_nElem) {
					case 4: {
						_readCache<GLfloat, cnv::Mat<2,2>>();
						break; }
					case 9:
						_readCache<GLfloat, cnv::Mat<3,3>>();
						break;
					case 16:
						_readCache<GLfloat, cnv::Mat<4,4>>();
						break;
					default:
						D_Assert0(false);
				}
			} else {
				if(_nElem == 1) {
					switch(_componentType) {
						case GL_BYTE:
							_readCache<GLbyte, cnv::Scalar>();
							break;
						case GL_UNSIGNED_BYTE:
							_readCache<GLubyte, cnv::Scalar>();
							break;
						case GL_SHORT:
							_readCache<GLshort, cnv::Scalar>();
							break;
						case GL_UNSIGNED_SHORT:
							_readCache<GLushort, cnv::Scalar>();
							break;
						default:
							_readCache<GLfloat, cnv::Scalar>();
					}
				} else if(_nElem == 2) {
					if(_bFloat) {
						_readCache<GLfloat, cnv::Vec<2>>();
					} else {
						_readCache<GLint, cnv::Vec<2>>();
					}
				} else if(_nElem == 3) {
					if(_bFloat) {
						_readCache<GLfloat, cnv::Vec<3>>();
					} else {
						_readCache<GLint, cnv::Vec<3>>();
					}
				} else {
					D_Assert0(_nElem == 4);
					if(_bFloat) {
						_readCache<GLfloat, cnv::Vec<4>>();
					} else {
						_readCache<GLint, cnv::Vec<4>>();
					}
				}
			}
			if(!_filter.empty() && _filterEnabled()) {
				visitor::Filter v(_filter.data());
				boost::apply_visitor(v, _cache);
			}
			_onCacheMaked(_cache);
		}
		return _cache;
	}
	namespace {
		using A = Accessor;
	}
	const A::Vec<GLbyte>& A::getAsByte() const {
		return boost::get<Vec<GLbyte>>(getData());
	}
	const A::Vec<GLubyte>& A::getAsUByte() const {
		return boost::get<Vec<GLubyte>>(getData());
	}
	const A::Vec<GLshort>& A::getAsShort() const {
		return boost::get<Vec<GLshort>>(getData());
	}
	const A::Vec<GLushort>& A::getAsUShort() const {
		return boost::get<Vec<GLushort>>(getData());
	}
	const A::Vec<GLfloat>& A::getAsFloat() const {
		return boost::get<Vec<GLfloat>>(getData());
	}
	const A::Vec<A::Mat2>& A::getAsMat2() const {
		return boost::get<Vec<Mat2>>(getData());
	}
	const A::Vec<A::Mat3>& A::getAsMat3() const {
		return boost::get<Vec<Mat3>>(getData());
	}
	const A::Vec<A::Mat4>& A::getAsMat4() const {
		return boost::get<Vec<Mat4>>(getData());
	}
	const A::Vec<A::IVec2>& A::getAsIVec2() const {
		return boost::get<Vec<IVec2>>(getData());
	}
	const A::Vec<A::IVec3>& A::getAsIVec3() const {
		return boost::get<Vec<IVec3>>(getData());
	}
	const A::Vec<A::IVec4>& A::getAsIVec4() const {
		return boost::get<Vec<IVec4>>(getData());
	}
	const A::Vec<A::Vec2>& A::getAsVec2() const {
		return boost::get<Vec<Vec2>>(getData());
	}
	const A::Vec<A::Vec3>& A::getAsVec3() const {
		return boost::get<Vec<Vec3>>(getData());
	}
	const A::Vec<A::Vec4>& A::getAsVec4() const {
		return boost::get<Vec<Vec4>>(getData());
	}
	namespace {
		namespace visitor {
			struct CnvInt32 {
				A::Vec<int32_t> result;
				template <class T, ENABLE_IF(lubee::is_number<T>{})>
				void operator()(const A::Vec<T>& t) {
					const auto len = t.size();
					result.resize(len);
					for(std::size_t i=0 ; i<len ; i++) {
						result[i] = t[i];
					}
				}
				template <class T, ENABLE_IF(!lubee::is_number<T>{})>
				void operator()(const A::Vec<T>&){ Assert0(false); }
				void operator()(boost::blank) { Assert0(false); }
			};
			struct MakeInfo {
				DataP_Unit result;
				template <class T>
				void operator()(const A::Vec<T>& t) {
					result = {
						DataP{
							.pointer = reinterpret_cast<uintptr_t>(t.data()),
							.length = t.size(),
						},
						.unitSize = sizeof(t[0]),
					};
				}
				void operator()(boost::blank) { Assert0(false); }
			};
		}
	}
	A::Vec<int32_t> A::cnvToInt32() const {
		visitor::CnvInt32 v;
		boost::apply_visitor(v, getData());
		return std::move(v.result);
	}
	DataP_Unit A::getDataP_Unit() const {
		visitor::MakeInfo v;
		boost::apply_visitor(v, getData());
		return v.result;
	}
}
