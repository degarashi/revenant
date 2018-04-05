#pragma once
#include "frea/fwd.hpp"
#include "frea/matrix.hpp"
#include "lubee/meta/enable_if.hpp"
#include "id.hpp"
#include "constant.hpp"
#include "lubee/meta/check_macro.hpp"
#include "spine/enum_t.hpp"
#include "spine/detect_type.hpp"
#include <memory>

namespace lubee {
	template <class T>
	struct Size;
	using SizeI = Size<int32_t>;
	using SizeF = Size<float>;
}
namespace beat {
	namespace g2 {
		class Pose;
	}
}
namespace spi {
	template <class T, class A>
	class ResMgr;
	template <class T, class K, class A>
	class ResMgrName;
}
namespace rev {
	struct IDebugGui;
	namespace debug {
		DEF_HASMETHOD(ToStr)
		template <class From, class To>
		struct EditProxy {
			From&	from;
			EditProxy(From& f):
				from(f)
			{}
		};
		template <class To, class From>
		EditProxy<From, To> MakeEditProxy(From& from) {
			return EditProxy<From,To>(from);
		}
		template <class T>
		void Show(IdPush, const T& t);
		template <class T>
		bool Edit(IdPush, T& t);
		namespace inner {
			template <class T, ENABLE_IF(std::is_floating_point_v<T>)>
			float ValueCnv(T);
			template <class T, ENABLE_IF(std::is_integral_v<T>)>
			int ValueCnv(T);
			template <class T>
			using ValueCnv_t = decltype(ValueCnv(std::declval<T>()));

			std::true_type ValueCnvB(float);
			std::true_type ValueCnvB(int);
			std::false_type ValueCnvB(...);
			template <class T>
			using ValueCnvB_t = decltype(ValueCnvB(std::declval<T>()));

			template <class V>
			void _ShowVector(const V&);
			template <class M>
			void _ShowMatrix(const M&);
			template <class V>
			bool _EditVector(V&);
			template <class M>
			bool _EditMatrix(M&);

			void _Show(bool b);
			void _Show(const char* s);
			void _Show(const std::string& s);
			template <class T, ENABLE_IF(DefaultDisplayTypes::Has<T>{} || std::is_integral<T>{} || std::is_floating_point<T>{})>
			void _Show(const T& t);
			void _Show(const lubee::SizeI& s);
			void _Show(const lubee::SizeF& s);
			void _Show(const beat::g2::Pose& p);
			void _Show(IDebugGui& g);
			template <class TAG, class V>
			void _Show(const frea::Angle<TAG,V>& a);
			template <class T, ENABLE_IF(frea::is_quaternion<T>{})>
			void _Show(const T& t);

			template <class V, ENABLE_IF(frea::is_vector<V>{})>
			void _Show(const V& v) {
				using V2 = frea::Vec_t<ValueCnv_t<typename V::value_t>, V::size, false>;
				_ShowVector(V2(v));
			}
			template <class M, ENABLE_IF(frea::is_matrix<M>{})>
			void _Show(const M& m) {
				using M2 = frea::Mat_t<ValueCnv_t<typename M::value_t>, M::dim_m, M::dim_n, false>;
				_ShowMatrix(M2(m));
			}

			template <class T, ENABLE_IF(HasMethod_ToStr_t<T>{})>
			void _Show(const T& t) {
				_Show(t.toStr());
			}
			template <class T, ENABLE_IF(spi::is_optional<T>{})>
			void _Show(const T& t) {
				if(t)
					_Show(*t);
				else
					_Show("(none)");
			}
			template <class T, class D>
			void _Show(const std::unique_ptr<T,D>& p) {
				if(p)
					_Show(*p);
				else
					_Show("(null)");
			}
			template <class T>
			void _Show(const std::shared_ptr<T>& p) {
				if(p)
					_Show(*p);
				else
					_Show("(null)");
			}
			template <class From, class To>
			void _Show(const EditProxy<From, To>& p) {
				_Show(static_cast<To>(p.from));
			}

			bool _Edit(bool& b);
			bool _Edit(float& f);
			bool _Edit(int32_t& i);
			template <class T, ENABLE_IF(std::is_integral<T>{})>
			bool _Edit(T& t);
			template <class T, ENABLE_IF(std::is_floating_point<T>{})>
			bool _Edit(T& t);
			bool _Edit(lubee::SizeI& s);
			bool _Edit(lubee::SizeF& s);
			bool _Edit(IDebugGui& g);
			bool _Edit(beat::g2::Pose& p);
			template <class TAG, class V>
			bool _Edit(frea::Angle<TAG,V>& a);

			template <class V, ENABLE_IF( frea::is_vector<V>{} && (!V::align && ValueCnvB_t<typename V::value_t>{}))>
			bool _Edit(V& v);
			template <class V, ENABLE_IF( frea::is_vector<V>{} && (V::align || !ValueCnvB_t<typename V::value_t>{}))>
			bool _Edit(V& v);
			template <class M, ENABLE_IF( frea::is_matrix<M>{} && (!M::align && ValueCnvB_t<typename M::value_t>{}))>
			bool _Edit(M& m);
			template <class M, ENABLE_IF( frea::is_matrix<M>{} && (M::align || !ValueCnvB_t<typename M::value_t>{}))>
			bool _Edit(M& m);

			template <class T, class A>
			bool _Edit(spi::ResMgr<T,A>& m);
			template <class T, class K, class A>
			bool _Edit(spi::ResMgrName<T,K,A>& m);
			template <class From, class To>
			bool _Edit(const EditProxy<From, To>& p) {
				To to = p.from;
				if(_Edit(to)) {
					p.from = to;
					return true;
				}
				return false;
			}
			bool _EditEnum(spi::Enum_t& value, spi::Enum_t len, const char* (*func)(spi::Enum_t));
			template <class T, ENABLE_IF(HasMethod_ToStr_t<T>{})>
			bool _Edit(T& t) {
				return _EditEnum(reinterpret_cast<spi::Enum_t&>(t.value), T::_Num, &T::ToStr);
			}
			template <class T, ENABLE_IF(spi::is_optional<T>{})>
			bool _Edit(const T& t) {
				if(t)
					return _Edit(*t);
				else {
					_Show("(none)");
					return false;
				}
			}
			template <class T, class D>
			bool _Edit(const std::unique_ptr<T,D>& p) {
				if(p)
					return _Edit(*p);
				else {
					_Show("(null)");
					return false;
				}
			}
			template <class T>
			bool _Edit(const std::shared_ptr<T>& p) {
				if(p)
					return _Edit(*p);
				else {
					_Show("(null)");
					return false;
				}
			}

			template <class T, ENABLE_IF(frea::is_quaternion<T>{})>
			bool _Edit(T& t);
			template <class V, ENABLE_IF_I( frea::is_vector<V>{} && (!V::align && ValueCnvB_t<typename V::value_t>{}))>
			bool _Edit(V& v) {
				return _EditVector(v);
			}
			template <class V, ENABLE_IF_I( frea::is_vector<V>{} && (V::align || !ValueCnvB_t<typename V::value_t>{}))>
			bool _Edit(V& v) {
				using V2 = frea::Vec_t<typename V::value_t, V::size, false>;
				auto proxy = MakeEditProxy<V2>(v);
				return _Edit(proxy);
			}
			template <class M, ENABLE_IF_I( frea::is_matrix<M>{} && (!M::align && ValueCnvB_t<typename M::value_t>{}))>
			bool _Edit(M& m) {
				return _EditMatrix(m);
			}
			template <class M, ENABLE_IF_I( frea::is_matrix<M>{} && (M::align || !ValueCnvB_t<typename M::value_t>{}))>
			bool _Edit(M& m) {
				using M2 = frea::Mat_t<typename M::value_t, M::dim_m, M::dim_n, false>;
				auto proxy = MakeEditProxy<M2>(m);
				return _Edit(proxy);
			}

			bool _Slider(int& i, int v_min, int v_max);
			template <class T, ENABLE_IF(std::is_integral<T>{})>
			bool _Slider(T& t, T v_min, T v_max, T step);
			bool _Slider(float& f, float v_min, float v_max);
			template <class T, ENABLE_IF(std::is_floating_point<T>{})>
			bool _Slider(T& t, T v_min, T v_max);
			template <class TAG, class V>
			bool _Slider(frea::Angle<TAG,V>& a, const frea::Angle<TAG,V>& v_min, const frea::Angle<TAG,V>& v_max);
			template <class V, ENABLE_IF(frea::is_vector<V>{})>
			bool _Slider(V& v, const typename V::value_t& v_min, const typename V::value_t& v_max);
		}
		template <class T>
		void Show(IdPush, const T& t) {
			inner::_Show(t);
		}
		template <class Itr>
		void Show(IdPush, Itr itr, const Itr itrE) {
			std::size_t idx=0;
			while(itr != itrE) {
				Show(idx++, *itr);
				++itr;
			}
		}
		template <class T>
		bool Edit(IdPush, T& t) {
			return inner::_Edit(t);
		}
		template <class T>
		bool EditIf(const bool edit, IdPush idp, T&& t) {
			if(edit)
				return Edit(std::move(idp), std::forward<T>(t));
			Show(std::move(idp), std::forward<T>(t));
			return false;
		}
		template <class Itr>
		bool Edit(IdPush, Itr itr, const Itr itrE) {
			bool ret = false;
			std::size_t idx=0;
			while(itr != itrE) {
				ret |= Edit(idx++, *itr);
				++itr;
			}
			return ret;
		}
		template <class T, class... Ts>
		bool Slider(IdPush, T& t, const Ts&... ts) {
			return inner::_Slider(t, ts...);
		}
	}
}
