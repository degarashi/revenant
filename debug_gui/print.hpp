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
	class Camera2D;
	class IGLTexture;
	class Texture_URI;
	class URI;
	class IdURI;
	class UserURI;
	class FileURI;
	class DataURI;
	struct Resource;
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
		namespace inner {
			void _Show(bool b);
			void _Show(const char* s);
			void _Show(const std::string& s);
			template <class T, ENABLE_IF(DefaultDisplayTypes::Has<T>{} || std::is_integral<T>{} || std::is_floating_point<T>{})>
			void _Show(const T& t);
			void _Show(const lubee::SizeI& s);
			void _Show(const lubee::SizeF& s);
			void _Show(const Resource& r);
			void _Show(const beat::g2::Pose& p);
			template <class TAG, class V>
			void _Show(const frea::Angle<TAG,V>& a);
			template <class V, ENABLE_IF(frea::is_vector<V>{})>
			void _Show(const V& v);
			template <class M, ENABLE_IF(frea::is_matrix<M>{})>
			void _Show(const M& m);
			template <class T, class A>
			void _Show(const spi::ResMgr<T,A>& m);
			template <class T, class K, class A>
			void _Show(const spi::ResMgrName<T,K,A>& m);
			template <class T, ENABLE_IF(HasMethod_ToStr_t<T>{})>
			void _Show(const T& t) {
				_Show(t.toStr());
			}
			void _Show(const URI& u);
			void _Show(const IdURI& u);
			void _Show(const UserURI& u);
			void _Show(const FileURI& u);
			void _Show(const DataURI& u);
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

			bool _Edit(bool& b);
			bool _Edit(float& f);
			bool _Edit(double& d);
			bool _Edit(int& i);
			bool _Edit(unsigned int& u);
			bool _Edit(lubee::SizeI& s);
			bool _Edit(lubee::SizeF& s);
			bool _Edit(Resource& r);
			bool _Edit(::rev::Camera2D& c);
			bool _Edit(::rev::IGLTexture& t);
			bool _Edit(::rev::Texture_URI& t);
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

			bool _Slider(Resource& r, ...);
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
		template <class Id, class T, class... Ts>
		bool Slider(const Id& id, T& t, const Ts&... ts) {
			const IdPush ip(id);
			return inner::_Slider(t, ts...);
		}
	}
}
