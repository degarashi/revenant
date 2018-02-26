#pragma once
#include "../../imgui/imgui.h"
#include "frea/matrix.hpp"
#include "../id.hpp"
#include "../constant.hpp"

namespace rev {
	namespace debug {
		namespace {
			const ImVec4 c_axisColor[4] = {
				{1,		0.5f,	0.5f,	1},
				{0.5f,	1,		0.5f,	1},
				{0.5f,	0.5f,	1,		1},
				{1,		0.5f,	1,		1},
			};
		}
		template <class V>
		class VectorC {
			private:
				const V& _v;
			public:
				VectorC(const V& v):
					_v(v)
				{}
				void show(const char* display_format = DefaultDisplayFormat(typename V::value_t()).c_str()) const {
					ImGui::Columns(V::size, "", false);
					for(int i=0 ; i<V::size ; i++) {
						const IdPush id(i);
						ImGui::TextColored(c_axisColor[i], display_format, _v.m[i]);
						ImGui::NextColumn();
					}
					ImGui::Columns(1);
				}
		};
		template <
			class V,
			ENABLE_IF(
				frea::is_vector<V>{} &&
				(std::is_same<typename V::value_t, float>{})
			)
		>
		class FVector : public VectorC<V> {
			private:
				V&	_v;
			public:
				FVector(V& v):
					VectorC<V>(v),
					_v(v)
				{}
				bool edit(const char* label, const float step=0.f, const float step_fast=0.f, const int decimal_precision=-1) {
					const std::function<bool (float*)> c_proc[4] = {
						[=](auto* f){ return ImGui::InputFloat(label, f, step, step_fast, decimal_precision); },
						[=](auto* f){ return ImGui::InputFloat2(label, f, decimal_precision); },
						[=](auto* f){ return ImGui::InputFloat3(label, f, decimal_precision); },
						[=](auto* f){ return ImGui::InputFloat4(label, f, decimal_precision); }
					};
					return c_proc[V::size-1](_v.m);
				}
				bool slider(const char* label, const float v_min, const float v_max,
							const char* display_format = DefaultDisplayFormat(float()).c_str(), const float power=1.f)
				{
					const std::function<bool (float*)> c_proc[4] = {
						[=](auto* f){ return  ImGui::SliderFloat(label, f, v_min, v_max, display_format, power); },
						[=](auto* f){ return  ImGui::SliderFloat2(label, f, v_min, v_max, display_format, power); },
						[=](auto* f){ return  ImGui::SliderFloat3(label, f, v_min, v_max, display_format, power); },
						[=](auto* f){ return  ImGui::SliderFloat4(label, f, v_min, v_max, display_format, power); },
					};
					return c_proc[V::size-1](_v.m);
				}
		};
		template <
			class V,
			ENABLE_IF(
				frea::is_vector<V>{} &&
				(std::is_same<typename V::value_t, int>{})
			)
		>
		class IVector : public VectorC<V> {
			private:
				V&	_v;
			public:
				IVector(V& v):
					VectorC<V>(v),
					_v(v)
				{}
				bool edit(const char* label, const int step=1, const int step_fast=100) {
					const std::function<bool (int*)> c_proc[4] = {
						[=](auto* i){ return ImGui::InputInt(label, i, step, step_fast); },
						[=](auto* i){ return ImGui::InputInt2(label, i); },
						[=](auto* i){ return ImGui::InputInt3(label, i); },
						[=](auto* i){ return ImGui::InputInt4(label, i); }
					};
					return c_proc[V::size-1](_v.m);
				}
				bool slider(const char* label, const int v_min, const int v_max, const char* display_format = DefaultDisplayFormat(int()).c_str()) {
					const std::function<bool (int*)> c_proc[4] = {
						[=](auto* i){ return ImGui::SliderInt(label, i, v_min, v_max, display_format); },
						[=](auto* i){ return ImGui::SliderInt2(label, i, v_min, v_max, display_format); },
						[=](auto* i){ return ImGui::SliderInt3(label, i, v_min, v_max, display_format); },
						[=](auto* i){ return ImGui::SliderInt4(label, i, v_min, v_max, display_format); }
					};
					return c_proc[V::size-1](_v.m);
				}
		};
	}
}
