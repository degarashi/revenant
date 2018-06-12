#include "../../gl_texture.hpp"
#include "../../imgui/imgui.h"
#include "../../imgui_sdl2.hpp"
#include "../entry_field.hpp"
#include "../child.hpp"
#include "../../uri.hpp"

namespace rev {
	const char* Texture_Mem::getDebugName() const noexcept {
		return "Texture_Mem";
	}

	bool IGLTexture::property(const bool /*edit*/) {
		/*
		auto field = debug::EntryField("IGLTexture", edit);
		field.show( "OpenGL Id", _idTex);
		{
			const auto p0 = debug::MakeEditProxy<bool>(_iLinearMag);
			const auto p1 = debug::MakeEditProxy<bool>(_iLinearMin);
			field.entry("Linear-Mag", p0);
			field.entry("Linear-Min", p1);
		}
		field.entry("Aniso-Coeff", _coeff);
		if(field.entry("Wrap-S", _wrapS) | field.entry("Wrap-T", _wrapT))
			setUVWrap(_wrapS, _wrapT);
		field.show("MipState", _mipLevel);
		field.show("Size", _size);
		field.show("Cube", _texFlag != GL_TEXTURE_2D);
		field.show(
			"CubeFace",
			(_faceFlag == GL_TEXTURE_2D) ?
			-1 : int(_faceFlag - GL_TEXTURE_CUBE_MAP_POSITIVE_X)
		);
		if(_format)
			field.show("Format", GLFormat::QueryEnumString(*_format));
		else
			field.show("Format", "(none)");

		ImGui::Columns(1);
		if(const auto c = debug::ChildPush("texture", 0, true, ImGuiWindowFlags_HorizontalScrollbar)) {
			ImGui::Image(
				mgr_gui.storeTexture(std::static_pointer_cast<const IGLTexture>(shared_from_this())),
				ImVec2(_size.width, _size.height)
			);
		}
		return field.modified();
		*/
		return false;
	}
	void IGLTexture::summary() const {
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
		ImGui::Image(mgr_gui.storeTexture(std::static_pointer_cast<const IGLTexture>(shared_from_this())), s);
	}
	bool Texture_URI::property(const bool edit) {
		auto field = debug::EntryField(getDebugName(), edit);
		field.entry("uri", _uri);
		ImGui::Columns(1);
		return field.modified() | IGLTexture::property(edit);
	}
	const char* Texture_URI::getDebugName() const noexcept {
		return "Texture_URI";
	}

	const char* Texture_CubeURI::getDebugName() const noexcept {
		return "Texture_CubeURI";
	}
}
