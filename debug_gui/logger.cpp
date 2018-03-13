#include "logger.hpp"
#include "../imgui/imgui.h"
#include "window.hpp"
#include "child.hpp"
#include "pooled_output.hpp"
#include "lubee/meta/countof.hpp"
#include "style.hpp"
#include "id.hpp"

namespace rev {
	Logger::View::View():
		filter{},
		scrBottom(true)
	{
		for(auto& s : show.type)
			s = true;
	}
	void Logger::_filter() {
		_filteredIndex.clear();
		ImGuiTextFilter filter(_view.filter);
		int idx = 0;
		for(auto& e : _entry) {
			if(_view.show.type[e.type] && filter.PassFilter(e.msg.c_str())) {
				_filteredIndex.emplace_back(idx);
			}
			++idx;
		}
	}
	Logger::Logger() {
		clear();
	}
	void Logger::clear() {
		_entry.clear();
		_selected = -1;
		_clearFlag = false;
		_filterFlag = false;
		_filteredIndex.clear();
	}
	void Logger::update() {
		if(_clearFlag)
			clear();
		else if(_filterFlag) {
			_filter();
			_filterFlag = false;
		}

		ImGuiTextFilter filter(_view.filter);
		int idx = _entry.size();
		auto& out = static_cast<log::PooledRevOutput&>(*lubee::log::Log::GetOutput());
		const auto ent = out.takeOut();
		for(auto& e : ent) {
			if(_view.show.type[e.type] && filter.PassFilter(e.msg.c_str())) {
				_filteredIndex.emplace_back(idx);
			}
			++idx;
			_entry.emplace_back(std::move(e));
		}
	}
	namespace {
		const ImVec4 c_textColor[lubee::log::Type::_Num] = {
			{1.0f,0.2f,0.2f,1},
			{1.0f,0.5f,0.5f,1},
			{1.0f,1.0f,1.0f,1},
		};
		const char* c_typeName[lubee::log::Type::_Num] = {
			"Error",
			"Info",
			"Verbose",
		};
	}
	void Logger::drawGUI(bool *const p_open) const {
		if(const auto w = debug::WindowPush("Log", p_open)) {
			const auto chkbox = [this](const auto typ){
				const auto s = debug::StylePush(ImGuiCol_Text, c_textColor[typ]);
				if(ImGui::Checkbox(c_typeName[typ], &_view.show.type[typ]))
					_filterFlag = true;
			};
			chkbox(Type::Verbose);
			ImGui::SameLine(0, 16);
			chkbox(Type::Info);
			ImGui::SameLine(0, 16);
			chkbox(Type::Error);
			ImGui::Separator();
			if(ImGui::InputText("filter", _view.filter, countof(_view.filter)))
				_filterFlag = true;
			ImGui::Text("showing %d of %d", int(_filteredIndex.size()), int(_entry.size()));
			// ログ詳細を表示するスペース
			constexpr int InfoSpace = 200;
			if(const auto c = debug::ChildPush("log", {0,-ImGui::GetItemsLineHeightWithSpacing() - InfoSpace}, true)) {
				const auto space = debug::StylePush(ImGuiStyleVar_ItemSpacing, ImVec2{0,0});
				if(!_filteredIndex.empty()) {
					ImGuiListClipper clipper(_filteredIndex.size());
					// 最大2行まで表示
					constexpr int RowWidth = 2;
					const float lineHeight = ImGui::GetTextLineHeight() * RowWidth;
					while(clipper.Step()) {
						for(int i=clipper.DisplayStart ; i<clipper.DisplayEnd ; i++) {
							const int idx = _filteredIndex[i];
							auto& e = _entry[idx];
							const auto s = debug::StylePush(ImGuiCol_Text, c_textColor[e.type]);
							const debug::IdPush idp(idx);
							if(ImGui::Selectable(e.msg.c_str(), idx==_selected, 0, {0, lineHeight})) {
								_selected = idx;
							}
						}
					}
					if(!ImGui::IsMouseDown(0) && _view.scrBottom)
						ImGui::SetScrollHere(1.f);
				}
			}
			if(ImGui::Button("clear")) {
				_clearFlag = true;
			}
			ImGui::SameLine(0, 20);
			ImGui::Checkbox("scroll to bottom", &_view.scrBottom);

			if(const auto c = debug::ChildPush("info", {0,0}, true, ImGuiWindowFlags_HorizontalScrollbar)) {
				if(!_clearFlag && _selected >= 0) {
					auto& e = _entry[_selected];
					ImGui::TextUnformatted(c_typeName[e.type]);
					ImGui::Separator();
					ImGui::TextUnformatted(e.msg.c_str());
				}
			}
		}
	}
}
