#include "../../dc/node.hpp"
#include "../../imgui/imgui.h"
#include "../../handle/gui.hpp"
#include "../treeview.hpp"
#include "../indent.hpp"
#include "../group.hpp"
#include "../textfilter.hpp"

namespace rev::dc {
	const char* TfRoot::getDebugName() const noexcept {
		return "TfRoot";
	}
	bool TfRoot::property(const bool) {
		ImGui::TextUnformatted("Filter");

		// フィルタタイプ選択
		static std::size_t type = 0;
		const char* c_typeName[] = {
			"By ID",
			"By Name",
			"By JointName"
		};
		ImGui::PushItemWidth(150.f);
		if(ImGui::BeginCombo("##FilterType", c_typeName[type])) {
			std::size_t idx = 0;
			for(auto& c : c_typeName) {
				if(ImGui::Selectable(c)) {
					type = idx;
				}
				++idx;
			}
			ImGui::EndCombo();
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();

		std::vector<TfNode*> result;
		{
			const auto& node = this->getNode();
			// フィルタ文字列入力欄
			if(type == 0) {
				const auto _ = debug::GroupPush();
				static int num = 0;
				ImGui::InputInt("##id", &num);
				for(auto& n : node) {
					n->iterateDepthFirst<true>([&result](auto& n, int){
						if(int(n.id) == num) {
							result.emplace_back(&n);
						}
						return spi::Iterate::StepIn;
					});
				}
			} else if(type == 1) {
				const auto id = ImGui::GetID("byName");
				debug::TextFilter filter(id);
				if(!filter.buffer().empty()) {
					for(auto& n : node) {
						n->iterateDepthFirst<true>([&filter, &result](auto& n, int){
							if(filter.PassFilter(n.userName.c_str())) {
								result.emplace_back(&n);
							}
							return spi::Iterate::StepIn;
						});
					}
				}
			} else {
				const auto id = ImGui::GetID("byJointName");
				debug::TextFilter filter(id);
				if(!filter.buffer().empty()) {
					for(auto& n : node) {
						n->iterateDepthFirst<true>([&filter, &result](auto& n, int){
							if(n.jointName && filter.PassFilter(n.jointName->c_str())) {
								result.emplace_back(&n);
							}
							return spi::Iterate::StepIn;
						});
					}
				}
			}
		}

		TfNode* result_select = nullptr;
		if(!result.empty()){
			// フィルター結果表示
			const auto len = result.size();
			ImGui::Text("Filter Result (%ld)", len);
			auto* st = ImGui::GetStateStorage();
			int* cur = st->GetIntRef(ImGui::GetID("result_cursor"), 0);
			std::vector<std::string>	items_s(len);
			for(std::size_t i=0 ; i<len ; ++i) {
				auto* r = result[i];
				items_s[i] = r->summary_str();
			}
			std::vector<const char*>	items(len);
			for(std::size_t i=0 ; i<len ; ++i) {
				items[i] = items_s[i].c_str();
			}

			if(ImGui::ListBox("##Result", cur, items.data(), items.size())) {
				result_select = result[*cur];
			}
		} else {
			ImGui::TextUnformatted("No result found");
		}
		ImGui::Separator();

		struct TreeV : debug::TreeView_Query {
			HTfNode					node;
			TfNode*					selected;
			std::vector<TfNode*>	open;

			TreeV(const HTfNode& n, TfNode* select):
				node(n),
				selected(select)
			{
				bool first = false;
				while(select) {
					if(!first)
						first = true;
					else
						open.emplace_back(select);
					select = select->getParent().get();
				}
			}
			bool _isOpened(TfNode* node) const {
				return std::count(open.cbegin(), open.cend(), node) > 0;
			}
			void onDown() override {
				node = node->getChild();
				D_Assert0(node);
			}
			void onUp() override {
				node = node->getParent();
				D_Assert0(node);
			}
			bool onSibling() override {
				if(const auto sib = node->getSibling()) {
					node = sib;
					return true;
				}
				return false;
			}
			HDbg getNode() const override {
				return node;
			}
			bool isLeaf() const override {
				return !node->getChild();
			}
			bool isOpened() const override {
				return _isOpened(node.get());
			}
			bool isSelected() const override {
				return node.get() == selected;
			}
		};
		auto& node = getNode();
		std::vector<TreeV> v2;
		for(auto& n : node)
			v2.emplace_back(n, result_select);
		TreeViewRange(v2.begin(), v2.end(), true);
		return false;
	}
}
