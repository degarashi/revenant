#pragma once
#include "../gl_format.hpp"
#include "data_pair.hpp"
#include "frea/matrix.hpp"
#include "json_types.hpp"

namespace rev::gltf {
	class Accessor {
		protected:
			using Size = std::size_t;
			using Size_OP = spi::Optional<Size>;

			using Vec2 = frea::Vec2;
			using Vec3 = frea::Vec3;
			using Vec4 = frea::Vec4;
			using IVec2 = frea::IVec2;
			using IVec3 = frea::IVec3;
			using IVec4 = frea::IVec4;
			using Mat2 = frea::Mat2;
			using Mat3 = frea::Mat3;
			using Mat4 = frea::Mat4;

			static struct Temporary_t {} Temporary;
			Accessor(Temporary_t, const Accessor& self);

		public:
			template <class T>
			using Vec = std::vector<T>;
			using Cache = boost::variant<
				boost::blank,
				Vec<GLbyte>,
				Vec<GLubyte>,
				Vec<GLshort>,
				Vec<GLushort>,
				Vec<GLfloat>,
				Vec<Vec2>,
				Vec<IVec2>,
				Vec<Vec3>,
				Vec<IVec3>,
				Vec<Vec4>,
				Vec<IVec4>,
				Vec<Mat2>,
				Vec<Mat3>,
				Vec<Mat4>
			>;
		private:
			using FilterP = std::pair<double,double>;
			using Filter = Vec<FilterP>;
			mutable Cache		_cache;
		public:
			GLTypeFmt		_componentType;
			Size			_byteOffset,
							_count;
			// 値が取り得る範囲 (length == nElem)
			Filter			_filter;
			// 1要素あたりの数値ペア数
			Size			_nElem;
			bool			_bMatrix,
							_bFloat;

		private:
			virtual Size_OP _getByteStride() const noexcept = 0;
			virtual DataP _getBufferData() const = 0;
			virtual bool _filterEnabled() const noexcept = 0;
			virtual void _onCacheMaked(Cache&) const {}
			template <class BuffType, class Cnv>
			void _readCache() const;
		public:
			Accessor(const JValue& v);

			const Cache& getData() const;
			const Vec<GLbyte>& getAsByte() const;
			const Vec<GLubyte>& getAsUByte() const;
			const Vec<GLshort>& getAsShort() const;
			const Vec<GLushort>& getAsUShort() const;
			const Vec<GLfloat>& getAsFloat() const;
			const Vec<Mat2>& getAsMat2() const;
			const Vec<Mat3>& getAsMat3() const;
			const Vec<Mat4>& getAsMat4() const;
			const Vec<IVec2>& getAsIVec2() const;
			const Vec<IVec3>& getAsIVec3() const;
			const Vec<IVec4>& getAsIVec4() const;
			const Vec<Vec2>& getAsVec2() const;
			const Vec<Vec3>& getAsVec3() const;
			const Vec<Vec4>& getAsVec4() const;

			Vec<int32_t> cnvToInt32() const;
			DataP_Unit getDataP_Unit() const;
	};
}
