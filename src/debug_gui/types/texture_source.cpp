#include "../../gl/texture/uri.hpp"
#include "../../gl/texture/mem.hpp"
#include "../../gl/texture/filter.hpp"
#include "../../gl/resource.hpp"
#include "../../imgui/imgui.h"
#include "../../imgui_sdl2.hpp"
#include "../entry_field.hpp"
#include "../child.hpp"
#include "../../uri/uri.hpp"

namespace rev {
	bool TextureSource::property(const bool edit) {
		auto field = debug::EntryField("TextureSource", edit);
		field.show( "OpenGL Id", _idTex);
		field.show("Size", _size);
		field.show("Cube", getTextureFlag() != GL_TEXTURE_2D);
		field.show(
			"CubeFace",
			(getFaceFlag() == GL_TEXTURE_2D) ?
			-1 : int(getFaceFlag() - GL_TEXTURE_CUBE_MAP_POSITIVE_X)
		);
		if(_format)
			field.show("Format", GLFormat::QueryEnumString(*_format));
		else
			field.show("Format", "(none)");

		ImGui::Columns(1);
		if(const auto c = debug::ChildPush("texture", 0, true, ImGuiWindowFlags_HorizontalScrollbar)) {
			auto filter = mgr_gl.createTexFilter();
			ImGui::Image(
				mgr_gui.storeTexture(
					mgr_gl.attachTexFilter(
						shared_from_this(),
						filter
					)
				),
				ImVec2(_size.width, _size.height)
			);
		}
		return field.modified();
	}
	void TextureSource::summary() const {
		ImVec2 s;
		constexpr float MaxSize = 256;
		const float m = std::max(_size.width, _size.height);
		if(m > MaxSize) {
			const float r = MaxSize / m;
			s.x = _size.width * r;
			s.y = _size.height * r;
		} else {
			s.x = _size.width;
			s.y = _size.height;
		}
		auto f = mgr_gl.createTexFilter();
		auto tex = mgr_gl.attachTexFilter(shared_from_this(), f);
		ImGui::Image(mgr_gui.storeTexture(tex), s);
	}
	bool TextureSrc_URI::property(const bool edit) {
		auto field = debug::EntryField(getDebugName(), edit);
		field.entry("uri", _uri);
		ImGui::Columns(1);
		return field.modified() | TextureSource::property(edit);
	}

	const char* TextureSrc_URI::getDebugName() const noexcept {
		return "TextureSrc_URI";
	}
	const char* TextureSrc_Mem2D::getDebugName() const noexcept {
		return "TextureSrc_Mem2D";
	}
	const char* TextureSrc_MemCube::getDebugName() const noexcept {
		return "TextureSrc_MemCube";
	}
	const char* TextureSrc_CubeURI::getDebugName() const noexcept {
		return "TextureSrc_CubeURI";
	}
}
