#pragma once
#include "gltf/v1/resource.hpp"
#include "gltf/v1/dataref.hpp"
#include "../../gl_format.hpp"
#include "gltf/v1/bufferview.hpp"
#include "../../ovr_functor.hpp"

namespace rev::gltf::v1 {
	namespace detail {
		using MinMaxP = std::pair<double, double>;
		using MinMaxV = std::vector<MinMaxP>;

		struct MinMax_Dummy {
			template <class V>
			V filter(const V val, const std::size_t) const noexcept {
				return val;
			}
		};
		template <class V, std::size_t Dim>
		struct MinMax {
			const MinMaxV*	range;
			MinMax(const MinMaxV& r):
				range(&r)
			{
				Assert0(r.size() >= Dim);
			}
			V filter(const V val, const std::size_t idx) const noexcept {
				auto& r = (*range)[idx];
				return lubee::Saturate<double>(val, r.first, r.second);
			}
		};

		template <class V, class Filter>
		struct Cnv_Scalar : Filter {
			using value_t = V;
			constexpr static std::size_t NElem = 1;
			using Filter::Filter;

			template <class Itr>
			value_t operator ()(Itr itr) const noexcept {
				return this->filter(*itr, 0);
			}
		};
		template <class V, std::size_t Dim, class Filter>
		struct Cnv_Vec : Filter {
			using value_t = frea::Vec_t<V, Dim, false>;
			constexpr static std::size_t NElem = Dim;
			using Filter::Filter;

			template <class Itr>
			value_t operator ()(Itr itr) const noexcept {
				value_t ret;
				for(std::size_t i=0 ; i<Dim ; i++) {
					ret.m[i] = this->filter(*itr, i);
					++itr;
				}
				return ret;
			}
		};
		template <class V, std::size_t DimM, std::size_t DimN, class Filter>
		struct Cnv_Mat : Filter {
			using value_t = frea::Mat_t<V, DimM, DimN, false>;
			constexpr static std::size_t NElem = DimM*DimN;
			using Filter::Filter;

			template <class Itr>
			value_t operator ()(Itr itr) const noexcept {
				value_t ret;
				std::size_t idx = 0;
				for(std::size_t i=0 ; i<DimM ; i++) {
					auto& vec = ret.m[i];
					for(std::size_t j=0 ; j<DimN ; j++) {
						vec.m[j] = this->filter(*itr, idx++);
						++itr;
					}
				}
				// column-major -> row-majorへの変換
				ret.transpose();
				return ret;
			}
		};
		template <class T>
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
			T& operator * () const noexcept {
				return *reinterpret_cast<T*>(cur);
			}
			bool operator == (const ItrSingle& itr) const noexcept {
				return cur == itr.cur;
			}
			bool operator != (const ItrSingle& itr) const noexcept {
				return !(this->operator ==(itr));
			}
		};
		template <class Cnv, class ItrS>
		struct Iterator {
			using value_t = typename Cnv::value_t;
			Cnv			cnv;
			ItrS		itr;
			std::size_t	stride;

			Iterator(const Cnv cnv, const ItrS itr, const std::size_t s):
				cnv(cnv),
				itr(itr)
			{
				if(s == 0)
					stride = Cnv::NElem;
				else
					stride = s;
			}
			value_t operator * () const noexcept {
				return cnv(itr);
			}
			Iterator& operator ++ () noexcept {
				itr += stride;
				return *this;
			}
			Iterator operator + (const std::size_t n) const noexcept {
				return {cnv, itr+n*stride, stride};
			}
			intptr_t operator - (const Iterator& i) const noexcept {
				const intptr_t diff = i.itr - itr;
				D_Assert0(diff % stride == 0);
				return diff / stride;
			}
			bool operator == (const Iterator& i) const noexcept {
				return itr == i.itr;
			}
			bool operator != (const Iterator& i) const noexcept {
				return !(this->operator ==(i));
			}
		};
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
	}
	//! BufferViewが指すバイナリデータを型情報付きで参照
	struct Accessor :
		Resource
	{
		// BufferViewに対するオフセット、間隔
		std::size_t		byteOffset,
						byteStride;
		// 数値型
		GLTypeFmt		componentType;
		//要素数
		std::size_t		count;

		// 値が取り得る範囲 (length == nElem)
		detail::MinMaxV		filter;

		// 参照元のバイナリデータ
		DRef_BufferView	bufferView;
		bool			bMatrix;
		// 1要素あたりの数値ペア数
		std::size_t		nElem;

		Accessor(const JValue& v, const IDataQuery& q);
		Type getType() const noexcept override;
		void filterValue(double* data) const noexcept;

		using Void_SP = std::shared_ptr<void>;
		mutable Void_SP	data_cached;
		template <class T>
		using SVec = std::shared_ptr<std::vector<T>>;

		template <class T>
		auto getDataAs() const -> SVec<T> {
			SVec<T> ret;
			getData(
				OVR_Functor{
					[&ret](const SVec<T>& p){
						ret = p;
					},
					[](const auto&){
						AssertF("type mismatch");
					}
				}
			);
			return ret;
		}
		template <class CB>
		void getData(CB&& cb) const {
			if(!data_cached) {
				readValue([&dst=this->data_cached](auto itr, const auto itrE, auto* type){
					using Type = std::remove_pointer_t<decltype(type)>;
					auto data = std::make_shared<std::vector<Type>>();
					while(itr != itrE) {
						data->emplace_back(*itr);
						++itr;
					}
					dst = data;
				});
			}
			// 実際には読まないがタイプ判別に必要なのでreadValueを呼ぶ
			readValue([this, &cb](auto,auto, auto* type){
				using Type = std::remove_pointer_t<decltype(type)>;
				cb(std::static_pointer_cast<std::vector<Type>>(data_cached));
			});
		}
		template <class CB>
		void readValue(CB&& cb) const {
			detail::SelectByType(componentType, [this, &cb](auto type){
				this->readValueAs<decltype(type)>(cb);
			});
		}
		template <class T, class CB>
		void readValueAs(CB&& cb) const {
			auto bv = bufferView.data()->getBuffer();
			bv.first += byteOffset;
			const auto bytype_cb = [&cb, &bv, this](auto type){
				using Type = decltype(type);
				const detail::ItrSingle<Type> itr{bv.first, sizeof(Type)};
				const auto selectByFilter = [&cb, itr, count=this->count, &filter=this->filter, stride=this->byteStride](auto* cnvT, auto* cnvF) {
					const auto iterate = [&cb, itr, count, stride](auto cnv){
						const detail::Iterator from{
							cnv,
							itr,
							stride / sizeof(Type)
						};
						const auto to = from + count;
						cb(from, to, (typename decltype(from)::value_t*)nullptr);
					};
					if(filter.empty()) {
						using CnvF = std::remove_pointer_t<decltype(cnvF)>;
						iterate(CnvF{});
					} else {
						using CnvT = std::remove_pointer_t<decltype(cnvT)>;
						iterate(CnvT{filter});
					}
				};
				D_Assert0(lubee::IsInRange<std::size_t>(nElem, 1, 16));

				using detail::Cnv_Scalar,
					  detail::Cnv_Vec,
					  detail::Cnv_Mat,
					  detail::MinMax,
					  detail::MinMax_Dummy;
				if(nElem == 1) {
					selectByFilter(
						(Cnv_Scalar<T, MinMax<T, 1>>*)nullptr,
						(Cnv_Scalar<T, MinMax_Dummy>*)nullptr
					);
				} else {
					if(bMatrix) {
						if(nElem == 2) {
							selectByFilter(
								(Cnv_Mat<T,2,2, MinMax<T,4>>*)nullptr,
								(Cnv_Mat<T,2,2, MinMax_Dummy>*)nullptr
							);
						} else if(nElem == 3) {
							selectByFilter(
								(Cnv_Mat<T,3,3, MinMax<T,9>>*)nullptr,
								(Cnv_Mat<T,3,3, MinMax_Dummy>*)nullptr
							);
						} else {
							selectByFilter(
								(Cnv_Mat<T,4,4, MinMax<T,16>>*)nullptr,
								(Cnv_Mat<T,4,4, MinMax_Dummy>*)nullptr
							);
						}
					} else {
						if(nElem == 2) {
							selectByFilter(
								(Cnv_Vec<T,2, MinMax<T,2>>*)nullptr,
								(Cnv_Vec<T,2, MinMax_Dummy>*)nullptr
							);
						} else if(nElem == 3) {
							selectByFilter(
								(Cnv_Vec<T,3, MinMax<T,3>>*)nullptr,
								(Cnv_Vec<T,3, MinMax_Dummy>*)nullptr
							);
						} else {
							selectByFilter(
								(Cnv_Vec<T,4, MinMax<T,4>>*)nullptr,
								(Cnv_Vec<T,4, MinMax_Dummy>*)nullptr
							);
						}
					}
				}
			};
			detail::SelectByType(componentType, bytype_cb);
		}
	};
}
