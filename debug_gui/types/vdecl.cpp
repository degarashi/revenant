#include "../../vdecl.hpp"
#include "../entry_field.hpp"
#include "../../gl_format.hpp"
#include "../sstream.hpp"
#include "../tree.hpp"

namespace rev {
	void VDecl::VDInfo::showAsRow() const {
		const auto next = [](){
			ImGui::NextColumn();
		};
		{
			StringStream s;
			s << sem.sem.toStr() << "[" << sem.index << "]";
			debug::Show(nullptr, s.output());
			next();
		}
		debug::Show(nullptr, streamId);
		next();
		debug::Show(nullptr, offset);
		next();
		debug::Show(nullptr, GLFormat::QueryEnumString(elemFlag));
		next();
		debug::Show(nullptr, bNormalize);
		next();
		debug::Show(nullptr, elemSize);
		next();
	}
	void VDecl::property(const CBProp& cb) const {
		constexpr char popup_id[] = "VDecl";
		const std::size_t n = _vdInfo.size();
		{
			auto f = debug::EntryField("VDecl", false, 7);
			ImGui::Separator();
			f.show("Id", "Semantic", "StreamId", "Offset", "Format", "Normalize", "Size");
			ImGui::Separator();
			for(std::size_t i=0 ; i<n ; i++) {
				const auto _ = debug::IdPush(i);
				const auto& vd = _vdInfo[i];
				ImGui::Selectable(std::to_string(i).c_str(), false, ImGuiSelectableFlags_SpanAllColumns);
				ImGui::OpenPopupOnItemClick(popup_id);
				ImGui::NextColumn();
				vd.showAsRow();
				if(ImGui::BeginPopup(popup_id)) {
					cb(vd);
					ImGui::EndPopup();
				}
			}
		}
		ImGui::Separator();
	}
}
