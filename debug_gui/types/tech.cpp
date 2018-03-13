#include "../../tech.hpp"
#include "../entry_field.hpp"
#include "../../gl_state.hpp"
#include "../indent.hpp"

namespace rev {
	bool Tech::property(const bool edit) {
		bool mod = false;
		auto f = debug::EntryField("Tech", edit);
		f.show("Name", getName());
		{
			const auto _ = debug::ColumnPush(1);
			if(ImGui::CollapsingHeader("Enable / Disable")) {
				const auto _ = debug::IndentPush();
				int idx = 0;
				for(auto& a : _setting) {
					if(a->getType() == GLState::Type::Boolean) {
						const auto __ = debug::IdPush(idx++);
						mod |= a->property(edit);
					}
				}
			}
			if(ImGui::CollapsingHeader("State")) {
				const auto _ = debug::IndentPush();
				int idx = 0;
				for(auto& a : _setting) {
					if(a->getType() == GLState::Type::Value) {
						const auto __ = debug::IdPush(idx++);
						mod |= a->property(edit);
					}
				}
			}
			if(ImGui::CollapsingHeader("VSemantics")) {
				const auto _ = debug::IndentPush();
				{
					const auto __ = debug::ColumnPush(2);
					ImGui::TextUnformatted("AttributeId");
					ImGui::NextColumn();
					ImGui::TextUnformatted("Semantic");
					ImGui::NextColumn();
					ImGui::Separator();
					for(auto& a : _vattr) {
						debug::Show(nullptr, a.attrId);
						ImGui::NextColumn();
						ImGui::Text("%s[%d]", a.sem.sem.toStr(), a.sem.index);
						ImGui::NextColumn();
					}
				}
				ImGui::Separator();
			}
		}
		return mod | f.modified();
	}
}
