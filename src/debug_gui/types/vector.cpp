#include "vector.hpp"

namespace rev::debug::inner {
	using Vec1 = frea::Vec_t<float, 1, false>;
	using IVec1 = frea::Vec_t<int32_t, 1, false>;
	template <class V>
	void _ShowVector(const V& v) {
		VectorC<V>(v).show();
	}
	template void _ShowVector(const Vec1& v);
	template void _ShowVector(const frea::Vec2& v);
	template void _ShowVector(const frea::Vec3& v);
	template void _ShowVector(const frea::Vec4& v);
	template void _ShowVector(const IVec1& v);
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
	template bool _EditVector(Vec1& v);
	template bool _EditVector(frea::Vec2& v);
	template bool _EditVector(frea::Vec3& v);
	template bool _EditVector(frea::Vec4& v);
	template bool _EditVector(IVec1& v);
	template bool _EditVector(frea::IVec2& v);
	template bool _EditVector(frea::IVec3& v);
	template bool _EditVector(frea::IVec4& v);

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
	template bool _Slider(Vec1& v, const float& v_min, const float& v_max);
	template bool _Slider(frea::Vec2& v, const float& v_min, const float& v_max);
	template bool _Slider(frea::Vec3& v, const float& v_min, const float& v_max);
	template bool _Slider(frea::Vec4& v, const float& v_min, const float& v_max);
	template bool _Slider(frea::IVec2& v, const int32_t& v_min, const int32_t& v_max);
	template bool _Slider(frea::IVec3& v, const int32_t& v_min, const int32_t& v_max);
	template bool _Slider(frea::IVec4& v, const int32_t& v_min, const int32_t& v_max);
}
