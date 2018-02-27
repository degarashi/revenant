#include "vector.hpp"

namespace rev {
	namespace debug {
		namespace inner {
			template <class V, ENABLE_IF(frea::is_vector<V>{})>
			void _Show(const V& v) {
				VectorC<V>(v).show();
			}
			template void _Show(const frea::Vec2& v);
			template void _Show(const frea::Vec3& v);
			template void _Show(const frea::Vec4& v);
			template void _Show(const frea::AVec2& v);
			template void _Show(const frea::AVec3& v);
			template void _Show(const frea::AVec4& v);
			template void _Show(const frea::IVec2& v);
			template void _Show(const frea::IVec3& v);
			template void _Show(const frea::IVec4& v);

			template <class V>
			bool _Edit_2(V& v, std::true_type) {
				return FVector<V>(v).edit("");
			}
			template <class V>
			bool _Edit_2(V& v, std::false_type) {
				return IVector<V>(v).edit("");
			}
			template <class V, ENABLE_IF(frea::is_vector<V>{})>
			bool _Edit(V& v) {
				return _Edit_2(v, std::is_floating_point<typename V::value_t>{});
			}
			template bool _Edit<frea::Vec2>(frea::Vec2& v);
			template bool _Edit<frea::Vec3>(frea::Vec3& v);
			template bool _Edit<frea::Vec4>(frea::Vec4& v);
			template bool _Edit<frea::AVec2>(frea::AVec2& v);
			template bool _Edit<frea::AVec3>(frea::AVec3& v);
			template bool _Edit<frea::AVec4>(frea::AVec4& v);
			template bool _Edit<frea::IVec2>(frea::IVec2& v);
			template bool _Edit<frea::IVec3>(frea::IVec3& v);
			template bool _Edit<frea::IVec4>(frea::IVec4& v);

			template <class V>
			bool _Slider_2(V& v, const typename V::value_t& v_min, const typename V::value_t& v_max, std::true_type) {
				return FVector<V>(v).slider("", v_min, v_max);
			}
			template <class V>
			bool _Slider_2(V& v, const typename V::value_t& v_min, const typename V::value_t& v_max, std::false_type) {
				return IVector<V>(v).slider("", v_min, v_max);
			}
			template <class V, ENABLE_IF(frea::is_vector<V>{})>
			bool _Slider(V& v, const typename V::value_t& v_min, const typename V::value_t& v_max) {
				return _Slider_2(v, v_min, v_max, std::is_floating_point<typename V::value_t>{});
			}
			template bool _Slider<frea::Vec2>(frea::Vec2& v, const float& v_min, const float& v_max);
			template bool _Slider<frea::Vec3>(frea::Vec3& v, const float& v_min, const float& v_max);
			template bool _Slider<frea::Vec4>(frea::Vec4& v, const float& v_min, const float& v_max);
			template bool _Slider<frea::AVec2>(frea::AVec2& v, const float& v_min, const float& v_max);
			template bool _Slider<frea::AVec3>(frea::AVec3& v, const float& v_min, const float& v_max);
			template bool _Slider<frea::AVec4>(frea::AVec4& v, const float& v_min, const float& v_max);
		}
	}
}
