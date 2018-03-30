#include "vector.hpp"

namespace rev::debug::inner {
	template <class V>
	void _ShowVector(const V& v) {
		VectorC<V>(v).show();
	}
	template void _ShowVector(const frea::Vec2& v);
	template void _ShowVector(const frea::Vec3& v);
	template void _ShowVector(const frea::Vec4& v);
	template void _ShowVector(const frea::IVec2& v);
	template void _ShowVector(const frea::IVec3& v);
	template void _ShowVector(const frea::IVec4& v);

	template <class V>
	bool _EditVector(V& v) {
		if constexpr (std::is_floating_point_v<typename V::value_t>) {
			return FVector<V>(v).edit("");
		} else {
			return IVector<V>(v).edit("");
		}
	}
	template bool _EditVector<frea::Vec2>(frea::Vec2& v);
	template bool _EditVector<frea::Vec3>(frea::Vec3& v);
	template bool _EditVector<frea::Vec4>(frea::Vec4& v);
	template bool _EditVector<frea::IVec2>(frea::IVec2& v);
	template bool _EditVector<frea::IVec3>(frea::IVec3& v);
	template bool _EditVector<frea::IVec4>(frea::IVec4& v);

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
