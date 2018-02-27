#pragma once
#include "frea/fwd.hpp"
#include "frea/detect_type.hpp"
#include "lubee/meta/enable_if.hpp"
#include "id.hpp"

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
	struct Resource;
	namespace debug {
		namespace inner {
			void _Show(const Resource& r);
			void _Show(const ::rev::Camera2D& c);
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

			bool _Edit(Resource& r);
			bool _Edit(::rev::Camera2D& c);
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
