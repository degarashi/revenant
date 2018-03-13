#include "../../vdecl.hpp"
#include "../entry_field.hpp"
#include "../../gl_format.hpp"
#include "../sstream.hpp"
#include "../tree.hpp"

namespace rev {
	void VDecl::VDInfo::showAsRow() const {
		{
			StringStream s;
			s << sem.sem.toStr() << "[" << sem.index << "]";
			debug::Show(nullptr, s.output());
			ImGui::NextColumn();
		}
		debug::Show(nullptr, streamId);
		ImGui::NextColumn();
		debug::Show(nullptr, offset);
		ImGui::NextColumn();
		debug::Show(nullptr, GLFormat::QueryEnumString(elemFlag));
		ImGui::NextColumn();
		debug::Show(nullptr, bNormalize);
		ImGui::NextColumn();
		debug::Show(nullptr, elemSize);
		ImGui::NextColumn();
	}
	void VDecl::property() const {
		const int n = _vdInfo.size();
		{
			auto f = debug::EntryField("VDecl", false, 7);
			ImGui::Separator();
			f.show("Id", "Semantic", "StreamId", "Offset", "Format", "Normalize", "Size");
			ImGui::Separator();
			for(int i=0 ; i<n ; i++) {
				debug::Show(nullptr, i);
				ImGui::NextColumn();
				_vdInfo[i].showAsRow();
			}
		}
		ImGui::Separator();
	}
}
