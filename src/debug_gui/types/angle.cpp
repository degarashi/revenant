#include "frea/src/angle.hpp"
#include "../constant.hpp"
#include "../../imgui/imgui.h"

namespace rev {
	namespace debug {
		template <class = void>
		class AngleBase {
			protected:
				static bool s_radian;
			public:
				static void SwitchType() {
					s_radian ^= 1;
				}
				static void SetType(const bool radian) {
					s_radian = radian;
				}
				static std::string ModAngleFormat(const std::string& disp) {
					if(s_radian)
						return disp + " (rad)";
					else
						return disp + " (deg)";
				}
				static void ShowAngleType() {
					if(ImGui::IsItemClicked(1)) {
						ImGui::OpenPopup("switch-disp");
					}
					if(ImGui::BeginPopup("switch-disp")) {
						bool change = false;
						int idx = static_cast<int>(AngleBase<>::s_radian);
						change |= ImGui::RadioButton("Degree", &idx, 0);
						change |= ImGui::RadioButton("Radian", &idx, 1);
						if(change) {
							AngleBase<>::SetType(static_cast<bool>(idx));
						}
						ImGui::EndPopup();
					}
				}
		};
		template <class T>
		bool AngleBase<T>::s_radian = false;

		template <class A>
		class AngleC : public AngleBase<> {
			private:
				const A& _a;
			public:
				AngleC(const A& a):
					_a(a)
				{}
				void show(const char* display_format = DefaultDisplayFormat<typename A::value_t>) const {
					const auto fmt = ModAngleFormat(display_format);
					if(s_radian)
						ImGui::Text(fmt.c_str(), frea::RadF(_a).get());
					else
						ImGui::Text(fmt.c_str(), frea::DegF(_a).get());
				}
		};

		template <class A>
		class Angle : public AngleC<A> {
			private:
				using base_t = AngleC<A>;
				A&	_a;
			public:
				Angle(A& a):
					base_t(a),
					_a(a)
				{}
				bool edit(const char* label, const float step=0.f, const float step_fast=0.f) {
					using Deg = frea::DegF;
					using Rad = frea::RadF;
					const auto lb = base_t::ModAngleFormat(label);
					const auto edf = [step, step_fast](const char* label, auto& data){
						return ImGui::InputFloat(label, &data, step, step_fast, -1, ImGuiInputTextFlags_EnterReturnsTrue);
					};
					if(base_t::s_radian) {
						const float prev = Rad(_a).get();
						float tmp = prev;
						if(edf(lb.c_str(), tmp)) {
							if(std::abs(tmp - prev) > 1e-4f) {
								_a = Rad(tmp);
								return true;
							}
						}
					} else {
						const float prev = Deg(_a).get();
						float tmp = prev;
						if(edf(lb.c_str(), tmp)) {
							if(std::abs(tmp - prev) > 1e-4f) {
								_a = Deg(tmp);
								return true;
							}
						}
					}
					AngleBase<>::ShowAngleType();
					return false;
				}
				bool slider(const char* label, const A& v_min, const A& v_max,
							const char* display_format = DefaultDisplayFormat<typename A::value_t>) {
					using Deg = frea::DegF;
					using Rad = frea::RadF;
					const auto disp = base_t::ModAngleFormat(display_format);
					if(base_t::s_radian) {
						float tmp = Rad(_a).get();
						if(ImGui::SliderFloat(label, &tmp, Rad(v_min).get(), Rad(v_max).get(), disp.c_str())) {
							_a = Rad(tmp);
							return true;
						}
					} else {
						float tmp = Deg(_a).get();
						if(ImGui::SliderFloat(label, &tmp, Deg(v_min).get(), Deg(v_max).get(), disp.c_str())) {
							_a = Deg(tmp);
							return true;
						}
					}
					AngleBase<>::ShowAngleType();
					return false;
				}
		};

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
