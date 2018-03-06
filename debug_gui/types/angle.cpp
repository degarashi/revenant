#include "angle.hpp"

namespace rev {
	namespace debug {
		namespace inner {
			template <class TAG, class V>
			void _Show(const frea::Angle<TAG,V>& a) {
				AngleC<frea::Angle<TAG,V>>(a).show();
			}
			template void _Show<frea::Degree_t, float>(const frea::Angle<frea::Degree_t, float>& a);
			template void _Show<frea::Degree_t, double>(const frea::Angle<frea::Degree_t, double>& a);
			template void _Show<frea::Radian_t, float>(const frea::Angle<frea::Radian_t, float>& a);
			template void _Show<frea::Radian_t, double>(const frea::Angle<frea::Radian_t, double>& a);

			template <class TAG, class V>
			bool _Edit(frea::Angle<TAG,V>& a) {
				return Angle<frea::Angle<TAG,V>>(a).edit("");
			}
			template bool _Edit<frea::Degree_t, float>(frea::Angle<frea::Degree_t, float>& a);
			template bool _Edit<frea::Degree_t, double>(frea::Angle<frea::Degree_t, double>& a);
			template bool _Edit<frea::Radian_t, float>(frea::Angle<frea::Radian_t, float>& a);
			template bool _Edit<frea::Radian_t, double>(frea::Angle<frea::Radian_t, double>& a);

			template <class TAG, class V>
			bool _Slider(frea::Angle<TAG,V>& a, const frea::Angle<TAG,V>& v_min, const frea::Angle<TAG,V>& v_max) {
				return Angle<frea::Angle<TAG,V>>(a).slider("", v_min, v_max);
			}
			template bool _Slider<frea::Degree_t, float>(
				frea::Angle<frea::Degree_t, float>& a,
				const frea::Angle<frea::Degree_t, float>& v_min,
				const frea::Angle<frea::Degree_t, float>& v_max
			);
			template bool _Slider<frea::Radian_t, float>(
				frea::Angle<frea::Radian_t, float>& a,
				const frea::Angle<frea::Radian_t, float>& v_min,
				const frea::Angle<frea::Radian_t, float>& v_max
			);
			template bool _Slider<frea::Degree_t, double>(
				frea::Angle<frea::Degree_t, double>& a,
				const frea::Angle<frea::Degree_t, double>& v_min,
				const frea::Angle<frea::Degree_t, double>& v_max
			);
			template bool _Slider<frea::Radian_t, double>(
				frea::Angle<frea::Radian_t, double>& a,
				const frea::Angle<frea::Radian_t, double>& v_min,
				const frea::Angle<frea::Radian_t, double>& v_max
			);
		}
	}
}
