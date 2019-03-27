#pragma once
#include "../gl/format.hpp"
#include "data_pair.hpp"
#include "frea/src/matrix.hpp"
#include "json_types.hpp"
#include "../handle/opengl.hpp"
#include "lubee/src/compare.hpp"
#include "../dc/bbox.hpp"

namespace rev::gltf {
	class Accessor {
		public:
			template <class T>
			using Vec = std::vector<T>;
			struct Vb_P {
				HVb			vb;
				std::size_t	offset;
			};

		protected:
			using Size = std::size_t;
			using Size_OP = spi::Optional<Size>;
			using Cache = Vec<uint8_t>;

			static struct Temporary_t {} Temporary;
			Accessor(Temporary_t, const Accessor& self);
		private:
			struct IFilter {
				virtual void filter(uintptr_t data) noexcept = 0;
				virtual ~IFilter() {}
			};
			using Filter_S = std::shared_ptr<IFilter>;
			template <class V>
			struct Filter;
			template <class V>
			struct FilterG;

			// 値が取り得る範囲 (length == nElem)
			Filter_S		_filter;
			// (POSITION用)バウンディングボックス
			dc::BBox_Op		_bbox;
			mutable Cache	_cache;

			struct Iterator {
				Size		nElem,
							stride;
				uintptr_t	pointer;

				virtual ~Iterator() {}
				Iterator(Size nE, Size str, uintptr_t ptr);
				bool operator == (const Iterator& itr) const noexcept;
				bool operator != (const Iterator& itr) const noexcept;
				virtual Size readAndIncrement(uintptr_t dst) noexcept = 0;
			};
			template <class Unit>
			struct VecIterator;
			template <class Unit>
			struct MatIterator;
			using Iterator_U = std::unique_ptr<Iterator>;

		public:
			GLTypeFmt		_componentType;
			Size			_byteOffset,		// for v2.0: default=0
							_count;
			// 1要素あたりの数値ペア数
			Size			_nElem;
			bool			_bMatrix,
							_bFloat;

		private:
			virtual Size_OP _getByteStride() const noexcept = 0;
			virtual DataP _getBufferData() const = 0;
			virtual bool _filterEnabled() const noexcept = 0;
			virtual void _onCacheMaked(Cache&) const {}

			const Cache& _getCache() const;

			template <class Type>
			static void _MatrixTranspose0(
				void* data,
				const std::size_t dim,
				const std::size_t n
			) {
				constexpr auto Unit = sizeof(Type);
				auto data_i = reinterpret_cast<uintptr_t>(data);
				for(std::size_t i=0 ; i<n ; i++) {
					for(std::size_t j=0 ; j<dim ; j++) {
						for(std::size_t k=j+1 ; k<dim ; k++) {
							auto& d0 = *reinterpret_cast<Type*>(data_i + (j*dim + k)*Unit);
							auto& d1 = *reinterpret_cast<Type*>(data_i + (k*dim + j)*Unit);
							std::swap(d0, d1);
						}
					}
					data_i += dim * dim * Unit;
				}
			}
			// column-major -> row-majorへの変換
			static void _MatrixTranspose(void* data, const Size unit, const Size dim, const Size n);
			void _applyFilter() const;

		protected:
			template <class CB>
			static void _SelectByType(const GLTypeFmt fmt, CB&& cb) {
				switch(fmt) {
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
					case GL_UNSIGNED_INT:
						cb(GLuint{});
						break;
					case GL_FLOAT:
						cb(GLfloat{});
						break;
					default:
						Assert0(false);
				}
			}
			template <class CB>
			void _selectByType(CB&& cb) const {
				const int n = _nElem;
				if(_bMatrix) {
					D_Assert0(_bFloat && lubee::IsInRange(n, 2, 4));
					if(n == 2) {
						cb((const frea::Mat2*)nullptr);
					} else if(n == 3) {
						cb((const frea::Mat3*)nullptr);
					} else
						cb((const frea::Mat4*)nullptr);
				} else {
					D_Assert0(lubee::IsInRange(n, 1, 4));
					if(n == 1) {
						_SelectByType(_componentType, [&cb](auto type){
							cb((const decltype(type)*)nullptr);
						});
					} else if(n == 2) {
						if(_bFloat)
							cb((const frea::Vec2*)nullptr);
						else
							cb((const frea::IVec2*)nullptr);
					} else if(n == 3) {
						if(_bFloat)
							cb((const frea::Vec3*)nullptr);
						else
							cb((const frea::IVec3*)nullptr);
					} else {
						if(_bFloat)
							cb((const frea::Vec4*)nullptr);
						else
							cb((const frea::IVec4*)nullptr);
					}
				}
			}
		public:
			Accessor(const JValue& v);
			Iterator_U begin() const noexcept;
			Iterator_U end() const noexcept;
			template <class CB>
			void getData(CB&& cb) const {
				_selectByType([
					&cb,
					data = _getCache().data(),
					count=_count
				](auto* typ){
					cb(reinterpret_cast<decltype(typ)>(data), count);
				});
			}

			template <class T>
			struct Visitor : DataP_Unit {
				void operator()(const T* data, const std::size_t len) {
					length = len;
					pointer = reinterpret_cast<uintptr_t>(data);
					unitSize = sizeof(data[0]);
				}
				template <class Other>
				void operator()(const Other*, std::size_t) {
					D_Assert0(false);
				}
			};
			template <class T>
			DataP_Unit getAs() const {
				Visitor<T> visitor;
				getData(visitor);
				return visitor;
			}

			const dc::BBox_Op& getBBox() const;
			std::size_t getActualNElem() const;
			std::size_t getUnitSize() const;
			Vec<float> cnvToFloat() const;
			Vec<int32_t> cnvToInt32() const;
			DataP_Unit getDataP_Unit() const;
			virtual Vb_P getAsVb() const = 0;
			virtual bool isNormalized() const = 0;
			Size getByteStride() const noexcept;
	};
}
