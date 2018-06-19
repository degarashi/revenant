#pragma once
#include "../gl_format.hpp"
#include "data_pair.hpp"
#include "frea/matrix.hpp"
#include "json_types.hpp"
#include "../handle/opengl.hpp"
#include "lubee/compare.hpp"

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
			mutable Cache	_cache;
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
			void _matrixTranspose() const;
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

			std::size_t getActualNElem() const;
			std::size_t getUnitSize() const;
			Vec<float> cnvToFloat() const;
			Vec<int32_t> cnvToInt32() const;
			DataP_Unit getDataP_Unit() const;
			virtual Vb_P getAsVb() const = 0;
			Size getByteStride() const noexcept;
	};
}
