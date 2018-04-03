#pragma once
#include "column.hpp"
#include "../imgui/imgui.h"
#include "state_storage.hpp"
#include "../debuggui_if.hpp"
#include "id.hpp"
#include "spine/enum.hpp"
#include "textfilter.hpp"
#include "spine/optional.hpp"
#include "child.hpp"

namespace rev::debug {
	DefineEnumPair(ListViewFlag,
		((NarrowColumn)(0x01))
		((Filter)(0x02))
	);
	template <class Itr>
	void ListView(Itr itr, const Itr itrE, const ListViewFlag::value_t flag={}) {
		if(const auto _ = debug::ChildPush("listview", {0,0}, false)) {
			const auto clm = debug::ColumnPush(2);
			if(flag & ListViewFlag::NarrowColumn) {
				const float NarrowWidth = (flag & ListViewFlag::Filter ? 380 : 120);
				ImGui::SetColumnWidth(0, NarrowWidth);
			}
			spi::Optional<TextFilter> filter;
			if(flag & ListViewFlag::Filter) {
				filter = spi::construct(ImGui::GetID("filter"));
			}

			const auto id = ImGui::GetID("Left");
			using WDbg = std::weak_ptr<IDebugGui>;
			using St = debug::StateStorage<WDbg>;
			auto cur = St::Get(id);
			if(const auto _ = debug::ChildPush("Left", {0,0}, false, ImGuiWindowFlags_HorizontalScrollbar)) {
				std::size_t idx = 0;
				while(itr != itrE) {
					const auto _ = debug::IdPush(idx++);
					using HDbg = std::shared_ptr<IDebugGui>;
					HDbg obj = *itr;
					const auto str = obj->summary_str();
					if(!filter || filter->PassFilter(str.c_str())) {
						if(ImGui::Selectable(str.c_str(), cur==obj)) {
							cur = obj;
							St::Set(id, cur);
						}
					}
					++itr;
				}
			}
			ImGui::NextColumn();
			{
				const auto idp = debug::IdPush("Right");
				if(cur) {
					cur->property(false);
				}
			}
		}
	}
}
