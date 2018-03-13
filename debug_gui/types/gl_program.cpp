#include "../../gl_program.hpp"
#include "../../gl_shader.hpp"
#include "../popup.hpp"
#include "../entry_field.hpp"
#include "../resource_window.hpp"
#include "../spacing.hpp"
#include "../child.hpp"
#include "../group.hpp"
#include "../tree.hpp"

namespace rev {
	const char* GLProgram::getDebugName() const noexcept {
		return "GL_Program";
	}
	bool GLProgram::property(const bool edit) {
		using EF = debug::EntryField;
		auto f = EF("Program", edit);
		f.show("OpenGL Id", _idProg);
		ImGui::Spacing();

		const auto _ = debug::ColumnPush(1);
		const auto print = [edit](const char* name, const int n, auto&& cb){
			if(n > 0) {
				ImGui::TextUnformatted(name);
				{
					auto f = EF(name, edit, 6);
					ImGui::Separator();
					f.show("Id", "Name", "Type", "Dim", "Unsigned", "Cubed");
					ImGui::Separator();
					for(int i=0 ; i<n ; i++) {
						debug::Show(nullptr, i);
						ImGui::NextColumn();
						cb(i).showAsRow();
					}
				}
				ImGui::Separator();
			}
		};
		print("Uniforms", getNActiveUniform(), [this](const int i){ return getActiveUniform(i); });
		ImGui::Spacing();
		print("Attributes", getNActiveAttribute(), [this](const int i){ return getActiveAttribute(i); });

		ImGui::Spacing();
		ImGui::TextUnformatted("Shaders:");
		ImGui::Separator();

		bool mod = false;
		for(int i=0 ; i<ShType::_Num ; i++) {
			const auto _ = debug::IdPush(i);
			if(_shader[i]) {
				const auto* name = ShType::ToStr(i);
				if(ImGui::CollapsingHeader(name)) {
					const auto _ = debug::ChildPush(name, ImGui::GetTextLineHeight()*20);
					mod |= _shader[i]->property(edit);
				}
				if(ImGui::IsItemClicked(1)) {
					ImGui::OpenPopup(name);
				}
				if(const auto _ = debug::PopupPush(name)) {
					if(ImGui::Selectable("Open in new window...")) {
						debug::ResourceWindow::Add(_shader[i]);
					}
				}
			}
		}
		return mod | f.modified();
	}

	void GLParamInfo::showAsRow() const {
		debug::Show(nullptr, name);
		ImGui::NextColumn();
		GLSLFormatDesc::showAsRow();
	}
}
