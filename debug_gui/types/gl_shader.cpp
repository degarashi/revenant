#include "../../gl_shader.hpp"
#include "../entry_field.hpp"
#include "../child.hpp"

namespace rev {
	bool GLShader::guiEditor(bool) {
		auto f = debug::EntryField("Shader");
		f.show("OpenGL Id", _idSh);
		f.show("Shader Type", _flag);
		ImGui::Columns(1);
		const auto _ = debug::ChildPush("Source", 0, true, ImGuiWindowFlags_HorizontalScrollbar);
		ImGui::TextUnformatted(_source.c_str());
		return false;
	}
}
