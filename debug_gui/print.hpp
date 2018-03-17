#pragma once
#include "frea/fwd.hpp"
#include "frea/detect_type.hpp"
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
		template <class Id, class T>
		void Show(const Id& id, const T& t);
		template <class Id, class T>
		bool Edit(const Id& id, T& t);
		namespace inner {
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
			template <class V, ENABLE_IF(frea::is_vector<V>{})>
			void _Show(const V& v);
			template <class M, ENABLE_IF(frea::is_matrix<M>{})>
			void _Show(const M& m);
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
			void _Show(const EditProxy<From, To>&) {}
			template <class T, std::size_t N>
			void _Show(const std::array<T,N>& a) {
				for(std::size_t i=0 ; i<N ; i++) {
					Show(i, a[i]);
				}
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
			template <class V, ENABLE_IF(frea::is_vector<V>{})>
			bool _Edit(V& v);
			template <class M, ENABLE_IF(frea::is_matrix<M>{})>
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
			template <class T, std::size_t N>
			bool _Edit(std::array<T,N>& a) {
				bool mod = false;
				for(std::size_t i=0 ; i<N ; i++) {
					mod |= Edit(i, a[i]);
				}
				return mod;
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
		template <class Id, class T>
		void Show(const Id& id, const T& t) {
			const IdPush ip(id);
			inner::_Show(t);
		}
		template <class Id, class T>
		bool Edit(const Id& id, T& t) {
			const IdPush ip(id);
			return inner::_Edit(t);
		}
		template <class Id, class T>
		bool EditIf(const bool edit, const Id& id, T&& t) {
			if(edit)
				return Edit(id, std::forward<T>(t));
			Show(id, std::forward<T>(t));
			return false;
		}
		template <class Id, class T, class... Ts>
		bool Slider(const Id& id, T& t, const Ts&... ts) {
			const IdPush ip(id);
			return inner::_Slider(t, ts...);
		}
	}
}
