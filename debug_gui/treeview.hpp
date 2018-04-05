#pragma once
#include "../handle/gui.hpp"
#include "../debuggui_if.hpp"
#include "../imgui/imgui.h"
#include "child.hpp"
#include "column.hpp"
#include "id.hpp"
#include "state_storage.hpp"
#include "tree.hpp"

namespace rev::debug {
	struct TreeView_Query {
		virtual ~TreeView_Query() {}
		virtual void onDown() = 0;
		virtual void onUp() = 0;
		virtual bool onSibling() = 0;
		virtual HDbg getNode() const = 0;
		virtual bool isLeaf() const = 0;
		virtual bool isOpened() const = 0;
	};
	bool _TreeView(TreeView_Query& q, HDbg& selecting, bool& bSelect, const std::size_t id) {
		const auto _ = IdPush(id);
		const auto cur = q.getNode();
		const auto str = cur->summary_str();
		ImGuiTreeNodeFlags flag = ImGuiTreeNodeFlags_OpenOnArrow |
								ImGuiTreeNodeFlags_OpenOnDoubleClick;
		bool clicked = false;
		if(cur == selecting)
			flag |= ImGuiTreeNodeFlags_Selected;
		if(q.isLeaf()) {
			flag |= ImGuiTreeNodeFlags_Leaf;
			TreePush(str.c_str(), flag);
			clicked = ImGui::IsItemClicked();
		} else {
			if(q.isOpened())
				ImGui::SetNextTreeNodeOpen(true);
			if(const auto _ = TreePush(str.c_str(), flag)) {
				clicked = ImGui::IsItemClicked();
				q.onDown();
				std::size_t idx = 0;
				while(_TreeView(q, selecting, bSelect, idx++));
				q.onUp();
			} else
				clicked = ImGui::IsItemClicked();
		}
		if(!bSelect && clicked) {
			selecting = cur;
			bSelect = true;
			std::cout << "HIT" << std::endl;
		}
		return q.onSibling();
	}
	void TreeView(TreeView_Query& q) {
		if(const auto _ = debug::ChildPush("treeview", {0,0}, false)) {
			const auto clm = debug::ColumnPush(2);
			const auto id = ImGui::GetID("Left");
			using St = debug::StateStorage<WDbg>;
			auto cur = St::Get(id);
			if(const auto _ = debug::ChildPush("Left", {0,0}, false, ImGuiWindowFlags_HorizontalScrollbar)) {
				std::size_t idx = 0;
				if(q.getNode()) {
					bool select = false;
					while(_TreeView(q, cur, select, idx++));
				}
			}
			St::Set(id, cur);
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
