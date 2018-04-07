#include "../../tech.hpp"
#include "../entry_field.hpp"
#include "../../gl_state.hpp"
#include "../indent.hpp"
#include "../header.hpp"

namespace rev {
	bool Tech::property(const bool edit) {
		bool mod = false;
		auto f = debug::EntryField("Tech", edit);
		f.show("Name", getName());
		{
			const auto _ = debug::ColumnPush(1);
			if(const auto _ = debug::Header("DefaultParameter", getDefaultValue().getIdEntry().empty())) {
				auto& val = const_cast<UniformEnt&>(getDefaultValue());
				mod |= val.property(edit);
			}

			if(const auto _ = debug::Header("NoDefaultParameter", getNoDefaultValue().empty())) {
				const auto& p = getProgram();
				const auto& nodef = getNoDefaultValue();
				for(auto id : nodef) {
					const auto* name = p->getUniformName(id);
					D_Assert0(name);
					ImGui::TextUnformatted(name);
				}
			}
			std::size_t nBool=0,
						nValue=0;
			std::for_each(_setting.cbegin(), _setting.cend(),
				[&nBool, &nValue](const auto& s){
					if(s->getType() == GLState::Type::Value)
						++nValue;
					else
						++nBool;
				}
			);
			if(const auto _ = debug::Header("Enable / Disable", nBool==0)) {
				std::size_t idx = 0;
				for(auto& a : _setting) {
					if(a->getType() == GLState::Type::Boolean) {
						const auto __ = debug::IdPush(idx++);
						mod |= a->property(edit);
					}
				}
			}
			if(const auto _ = debug::Header("State", nValue==0)) {
				std::size_t idx = 0;
				for(auto& a : _setting) {
					if(a->getType() == GLState::Type::Value) {
						const auto __ = debug::IdPush(idx++);
						mod |= a->property(edit);
					}
				}
			}
			if(const auto _ = debug::Header("VSemantics", _vattr.empty())) {
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
			if(const auto _ = debug::Header("Program", !getProgram())) {
				debug::Show("Program", getProgram());
			}
		}
		return mod | f.modified();
	}
}
