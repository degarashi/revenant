#pragma once
#include "frea/matrix.hpp"
#include "frea/quaternion.hpp"
#include "frea/expquat.hpp"
#include "lubee/meta/typelist.hpp"
#include "upobj.hpp"
#include "handle/resource.hpp"
#include "lv_common.hpp"
#include <boost/variant.hpp>

namespace rev {
	using Vec2 = frea::Vec_t<float,2,false>;
	using Vec3 = frea::Vec_t<float,3,false>;
	using Vec4 = frea::Vec_t<float,4,false>;
	struct LCVec4 : Vec4 {
		int size;

		LCVec4(const Vec2& v):
			Vec4(v.convert<4>()),
			size(2)
		{}
		LCVec4(const Vec3& v):
			Vec4(v.convert<4>()),
			size(3)
		{}
		LCVec4(const Vec4& v):
			Vec4(v),
			size(4)
		{}
	};

	using Mat2 = frea::Mat2;
	using Mat3 = frea::Mat3;
	using Mat4 = frea::Mat4;
	struct LCMat4 : UPObj<Mat4> {
		using base_t = UPObj<Mat4>;
		int	size;

		LCMat4(const Mat2& m):
			base_t(m.convert<4,4>()),
			size(2)
		{}
		LCMat4(const Mat3& m):
			base_t(m.convert<4,4>()),
			size(3)
		{}
		LCMat4(const Mat4& m):
			base_t(m),
			size(4)
		{}
	};
}
namespace std {
	template <>
	struct hash<rev::LCVec4> {
		std::size_t operator()(const rev::LCVec4& v) const noexcept {
			return std::hash<rev::Vec4>()(v);
		}
	};
	template <>
	struct hash<rev::LCMat4> {
		std::size_t operator()(const rev::LCMat4& m) const noexcept {
			return std::hash<rev::Mat4>()(*m);
		}
	};
}

namespace rev {
	#define SEQ_LCVAR \
		(LuaNil)(bool)(const char*)(lua_Number) \
		(LCVec4)(LCMat4)(frea::Quat)(frea::ExpQuat)(frea::RadF)(frea::DegF) \
		(HRes)(WRes)(Lua_SP)(LCTable_SP)(void*)(lua_CFunction)(std::string)
	using LCVar = boost::variant<BOOST_PP_SEQ_ENUM(SEQ_LCVAR)>;
	using LCVar_Types = lubee::Types<BOOST_PP_SEQ_ENUM(SEQ_LCVAR)>;
	#undef SEQ_LCVAR

	class LCValue : public LCVar {
		private:
			template <int N>
			using IConst = std::integral_constant<int, N>;
			template <class... Args, int N, ENABLE_IF((N==sizeof...(Args)))>
			static void _TupleAsTable(LCTable_SP&, const std::tuple<Args...>&, IConst<N>) {}
			template <class... Args, int N, ENABLE_IF((N != sizeof...(Args)))>
			static void _TupleAsTable(LCTable_SP& tbl, const std::tuple<Args...>& t, IConst<N>);
			template <class... Args>
			static LCTable_SP _TupleAsTable(const std::tuple<Args...>& t);
			HRes _toSPtr() const;
		public:
			struct HashVisitor : boost::static_visitor<std::size_t> {
				template <class T>
				std::size_t operator()(const T& t) const {
					return std::hash<T>()(t);
				}
			};
			LCValue();
			LCValue(const LCValue& lc);
			LCValue(LCValue&& lcv);
			LCValue(lua_OtherNumber num);
			LCValue(lua_Integer num);
			LCValue(lua_IntegerU num);
			LCValue(lua_OtherIntegerU num);
			LCValue(lua_OtherInteger num);
			LCValue(const frea::DegD& d);
			LCValue(const frea::RadD& r);
			LCValue(const Vec2& v);
			LCValue(const Vec3& v);
			LCValue(const Vec4& v);
			LCValue(const Mat2& m);
			LCValue(const Mat3& m);
			LCValue(const Mat4& m);
			template <
				class T,
				ENABLE_IF((
					LCVar_Types::Has<std::decay_t<T>>{}
				))
			>
			LCValue(T&& t): LCVar(std::forward<T>(t)) {}
			template <class T, ENABLE_IF((!LCVar_Types::Has<T*>{}))>
			LCValue(T* ptr):
				LCValue(reinterpret_cast<void*>(ptr))
			{}

			// Tupleは配列に変換
			LCValue(std::tuple<>& t);
			LCValue(std::tuple<>&& t);
			LCValue(const std::tuple<>& t);
			template <class... Args>
			LCValue(std::tuple<Args...>& t);
			template <class... Args>
			LCValue(std::tuple<Args...>&& t);
			template <class... Args>
			LCValue(const std::tuple<Args...>& t);
			template <
				class V,
				ENABLE_IF((
					!LCVar_Types::Has<std::decay_t<V>>{} &&
					frea::is_vector<V>{}
				))
			>
			LCValue(const V& v):
				LCVar(frea::Vec_t<float,V::size, false>(v))
			{}
			LCValue& operator = (const LCValue& lcv);
			LCValue& operator = (LCValue&& lcv);
			// リソースの固有ハンドルは汎用へ読み替え
			template <
				class T,
				ENABLE_IF((
					!LCVar_Types::Has<std::shared_ptr<T>>{} &&
					std::is_base_of<Resource, T>{}
				))
			>
			LCValue(const std::shared_ptr<T>& sp):
				LCVar(std::static_pointer_cast<Resource>(sp))
			{}
			//! 中身を配列とみなしてアクセス
			/*! \param[in] n 0オリジンのインデックス */
			const LCValue& operator[] (int n) const;

			//! 任意のリソースハンドルへ変換して取り出す
			template <class H>
			auto toSPtr() const {
				return std::static_pointer_cast<H>(_toSPtr());
			}
			bool preciseCompare(const LCValue& lcv) const;
			bool operator == (const LCValue& lcv) const noexcept;
			bool operator != (const LCValue& lcv) const noexcept;
			//! Nilかfalseの場合にのみfalse, それ以外はtrueを返す
			explicit operator bool () const noexcept;
			void push(lua_State* ls) const;
			const char* toCStr() const;
			std::string toString() const;
			LuaType type() const;
			template <int N>
			auto toVector() const {
				return boost::get<LCVec4>(*this).convert<N>();
			}
			template <int M, int N>
			auto toMatrix() const {
				auto& m = *boost::get<LCMat4>(*this);
				return m.convert<M,N>();
			}
			template <class A>
			A toAngle() const {
				return A(boost::get<frea::RadF>(*this));
			}
	};
	std::ostream& operator << (std::ostream& os, const LCValue& lcv);
}
namespace std {
	template <>
	struct hash<rev::LCValue> {
		std::size_t operator()(const rev::LCValue& v) const {
			return boost::apply_visitor(rev::LCValue::HashVisitor(), static_cast<const rev::LCVar&>(v));
		}
	};
}
