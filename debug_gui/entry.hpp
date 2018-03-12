#pragma once
#include "print.hpp"
#include "../imgui/imgui.h"
#include "column.hpp"
#include "group.hpp"

namespace rev {
	namespace debug {
		namespace inner {
			inline int ShowEnt(const int id) {
				return id;
			}
			template <class T0, class... Ts>
			int ShowEnt(const int id, const T0& ent, const Ts&... ts) {
				{
					const auto _ = GroupPush();
					Show(id, ent);
				}
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
		class Entry {
			private:
				int			_id;
				ColumnPush	_cp;
				bool		_modify;

			public:
				Entry(const int initial, const int columns):
					_id(initial),
					_cp(columns),
					_modify(false)
				{}
				template <class... Ts>
				void show(const Ts&... ts) {
					_id = inner::ShowEnt(_id, ts...);
				}
				template <class... Ts>
				bool edit(Ts&... ts) {
					const auto ret = inner::EditEnt(_id, ts...);
					_modify |= ret.second;
					_id = ret.first;
					return ret.second;
				}
				template <class... Ts>
				bool entry(const bool b_edit, Ts&&... ts) {
					if(b_edit) {
						return edit(std::forward<Ts>(ts)...);
					} else {
						show(std::forward<Ts>(ts)...);
						return false;
					}
				}
				bool modified() const noexcept {
					return _modify;
				}
		};

	}
}
