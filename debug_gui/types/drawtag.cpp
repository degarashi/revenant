#include "../../drawtag.hpp"
#include "../entry_field.hpp"
#include "../group.hpp"
#include "../../tech_if.hpp"
#include "../../gl_texture.hpp"
#include "../resource_window.hpp"
#include "../sstream.hpp"
#include "../indent.hpp"

namespace rev {
	const char* DrawTag::getDebugName() const noexcept {
		return "DrawTag";
	}
	bool DrawTag::property(const bool edit) {
		bool mod = false;
		if(technique) {
			if(ImGui::CollapsingHeader("Technique")) {
				const auto _ = debug::IndentPush();
				mod |= technique->property(edit);
			}
		}
		if(primitive) {
			if(ImGui::CollapsingHeader("Primitive")) {
				const auto _ = debug::IndentPush();
				mod |= primitive->property(edit);
			}
		}
		ImGui::Spacing();

		auto f = debug::EntryField("DrawTag", edit);
		if(idTex[0] || idTex[1] || idTex[2] || idTex[3]) {
			ImGui::TextUnformatted("Texture");
			ImGui::NextColumn();
			{
				StringStream s;
				const auto _ = debug::GroupPush();
				for(int i=0 ; i<4 ; i++) {
					const auto _ = debug::IdPush(i);
					if(idTex[i]) {
						idTex[i]->summary();
						if(ImGui::IsItemClicked())
							debug::ResourceWindow::Add(idTex[i]);
					} else
						break;
				}
			}
			ImGui::NextColumn();
		}
		f.entry("Priority", priority);
		f.entry("Z-Offset", zOffset);
		return mod | f.modified();
	}
}
