#include "../../gl_buffer.hpp"
#include "../../imgui/imgui.h"
#include "../entry_field.hpp"
#include "../child.hpp"
#include "../spacing.hpp"

namespace rev {
	// -------------- GLBuffer --------------
	const char* GLVBuffer::getDebugName() const noexcept {
		return "VertexBuffer";
	}
	bool GLBuffer::property(const bool edit) {
		auto f = debug::EntryField("GLBuffer", edit);
		f.show( "OpenGL_Id", _idBuff);
		f.show( "BufferType", _buffType);
		f.show( "DrawType", _drawType);
		f.show( "Stride", _stride);
		f.show( "BackupSize", _buffSize);
		return f.modified();
	}
	const char* GLIBuffer::getDebugName() const noexcept {
		return "IndexBuffer";
	}

	// -------------- GLIBuffer --------------
	template <class T>
	bool GLIBuffer::_property(const int columns, const char* typestr, bool) {
		const auto ne = getNElem();
		using S = debug::Spacing;
		ImGui::SetNextWindowContentWidth(columns * 32);
		if(const auto _ = debug::ChildPush( "GLIBuffer", -(S::Text() + S::ItemSpacing()), true, ImGuiWindowFlags_HorizontalScrollbar)) {
			const auto c = debug::ColumnPush(columns, false);
			auto* data = static_cast<const T*>(_getBufferPtr());
			auto n = ne;
			int col = columns;
			while(n != 0) {
				ImGui::Text("%d", *data);
				if(--col == 0) {
					col = columns;
				}
				ImGui::NextColumn();
				++data;
				--n;
			}
		}
		ImGui::Text("%s:\t%d elements (%d bytes)", typestr, ne, getSize());
		return false;
	}
	bool GLIBuffer::property(const bool edit) {
		auto* st = ImGui::GetStateStorage();
		int* columns = st->GetIntRef(ImGui::GetID(""), 16);
		ImGui::TextUnformatted("Columns: ");
		ImGui::SameLine();
		debug::Slider(nullptr, *columns, 1, 32);
		switch(getStride()) {
			case sizeof(GLubyte):
				return _property<GLubyte>(*columns, "GLubyte", edit);
			case sizeof(GLushort):
				return _property<GLushort>(*columns, "GLushort", edit);
			case sizeof(GLuint):
				return _property<GLuint>(*columns, "GLuint", edit);
			default:
				ImGui::TextUnformatted("unknown ibuffer size type");
		}
		return false;
	}
}
