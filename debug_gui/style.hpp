#pragma once
#include <type_traits>
#include "../imgui/imgui.h"

namespace rev {
	namespace debug {
		class StylePush {
			private:
				void _init(const int) {}
				template <class... Ts>
				void _init(const int cur, const ImGuiStyleVar id, const ImVec2& v, const Ts&... ts) {
					ImGui::PushStyleVar(id, v);
					++_pop.value;
					_init(cur+1, ts...);
				}
				template <class... Ts>
				void _init(const int cur, const ImGuiStyleVar id, const float f, const Ts&... ts) {
					ImGui::PushStyleVar(id, f);
					++_pop.value;
					_init(cur+1, ts...);
				}
				template <class... Ts>
				void _init(const int cur, const ImGuiCol id, const ImVec4& c, const Ts&... ts) {
					ImGui::PushStyleColor(id, c);
					++_pop.color;
					_init(cur+1, ts...);
				}
				template <class... Ts>
				void _init(const int cur, const ImGuiCol id, const ImU32& c, const Ts&... ts) {
					ImGui::PushStyleColor(id, c);
					++_pop.color;
					_init(cur+1, ts...);
				}
				struct Pop {
					int value = 0,
						color = 0;

					void clear() {
						value = color = 0;
					}
				} _pop;

			public:
				template <class... Ts>
				StylePush(const Ts&... ts) {
					static_assert(sizeof...(Ts)%2 == 0, "");
					_init(0, ts...);
				}
				StylePush(StylePush&& s):
					_pop(s._pop)
				{
					s._pop.clear();
				}
				~StylePush() {
					if(_pop.value > 0)
						ImGui::PopStyleVar(_pop.value);
					if(_pop.color > 0)
						ImGui::PopStyleColor(_pop.color);
				}
		};
	}
}
