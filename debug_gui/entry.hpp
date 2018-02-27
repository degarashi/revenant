#pragma once
#include "print.hpp"
#include "../imgui/imgui.h"
#include "column.hpp"

namespace rev {
	namespace debug {
		namespace inner {
			inline int ShowEnt(const int id) {
				return id;
			}
			template <class T0, class... Ts>
			int ShowEnt(const int id, const T0& ent, const Ts&... ts) {
				Show(id, ent);
				ImGui::NextColumn();
				return ShowEnt(id+1, ts...);
			}
			inline std::pair<int,bool> EditEnt(const int id) {
				return std::make_pair(id, false);
			}
			template <class T0, class... Ts>
			std::pair<int, bool> EditEnt(const int id, T0& ent, Ts&... ts);
			template <class... Ts>
			std::pair<int,bool> EditEnt(const int id, const char* str, Ts&... ts) {
				ShowEnt(id, str);
				return EditEnt(id, ts...);
			}
			template <class T0, class... Ts>
			std::pair<int,bool> EditEnt(const int id, T0& ent, Ts&... ts) {
				const bool b = Edit(id, ent);
				ImGui::NextColumn();
				auto ret = EditEnt(id+1, ts...);
				ret.second |= b;
				return ret;
			}
		}
		struct Entry {
			int	id;
			ColumnPush	cp;
			bool modify;

			Entry(const int initial, const int columns):
				id(initial),
				cp(columns)
			{}
			template <class... Ts>
			void show(const Ts&... ts) {
				id = inner::ShowEnt(id, ts...);
			}
			template <class... Ts>
			bool edit(Ts&... ts) {
				const auto ret = inner::EditEnt(id, ts...);
				modify |= ret.second;
				id = ret.first;
				return ret.second;
			}
			bool modified() const noexcept {
				return modify;
			}
		};

	}
}
