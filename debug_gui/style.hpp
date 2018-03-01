#pragma once
#include <type_traits>
#include "../imgui/imgui.h"

namespace rev {
	namespace debug {
		template <int N>
		class StylePush {
			private:
				void _init(const int) {}
				template <class... Ts>
				void _init(const int cur, const ImGuiStyleVar id, const ImVec2& v, const Ts&... ts) {
					ImGui::PushStyleVar(id, v);
					_init(cur+1, ts...);
				}
				template <class... Ts>
				void _init(const int cur, const ImGuiStyleVar id, const float f, const Ts&... ts) {
					ImGui::PushStyleVar(id, f);
					_init(cur+1, ts...);
				}
				template <class... Ts>
				void _init(const int cur, const ImGuiStyleVar id, const ImVec4& c, const Ts&... ts) {
					ImGui::PushStyleColor(id, c);
					_init(cur+1, ts...);
				}
				template <class... Ts>
				void _init(const int cur, const ImGuiStyleVar id, const ImU32& c, const Ts&... ts) {
					ImGui::PushStyleColor(id, c);
					_init(cur+1, ts...);
				}
				bool _valid;

			public:
				template <class... Ts>
				StylePush(const Ts&... ts):
					_valid(true)
				{
					static_assert(sizeof...(Ts)/2 == N, "");
					_init(0, ts...);
				}
				StylePush(StylePush&& s):
					_valid(s._valid)
				{
					s._valid = false;
				}
				~StylePush() {
					if(_valid)
						ImGui::PopStyleVar(N);
				}
		};
		template <class... Ts>
		auto MakeStylePush(const Ts&... ts) {
			return StylePush<sizeof...(Ts)/2>(ts...);
		}
	}
}
