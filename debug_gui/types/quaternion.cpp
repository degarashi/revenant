#include "../print.hpp"
#include "frea/quaternion.hpp"

namespace rev::debug::inner {
	// とりあえずVector4で表示させとく
	template <class T, ENABLE_IF_I(frea::is_quaternion<T>{})>
	void _Show(const T& t) {
		_Show(t.asVec4());
	}
	template void _Show(const frea::Quat&);
	template void _Show(const frea::AQuat&);

	template <class T, ENABLE_IF_I(frea::is_quaternion<T>{})>
	bool _Edit(T& t) {
		auto tmp = t.asVec4();
		if(_Edit(tmp)) {
			for(std::size_t i=0 ; i<4 ; i++)
			t.m[i] = tmp.m[i];
		}
		return false;
	}
	template bool _Edit(frea::Quat&);
	template bool _Edit(frea::AQuat&);
}
