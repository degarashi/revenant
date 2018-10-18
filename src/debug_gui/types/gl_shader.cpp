#include "../../gl_shader.hpp"
#include "../entry_field.hpp"
#include "../child.hpp"

namespace rev {
	const char* GLShader::getDebugName() const noexcept {
		return "Shader";
	}
	bool GLShader::property(const bool edit) {
		auto f = debug::EntryField("Shader", edit);
		f.show("OpenGL Id", _idSh);
		f.show("Shader Type", _flag);
		ImGui::Columns(1);
		const auto _ = debug::ChildPush("Source", 0, true, ImGuiWindowFlags_HorizontalScrollbar);
		ImGui::TextUnformatted(_source.c_str());
		return f.modified();
	}
}
