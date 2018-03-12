#include "../../gl_texture.hpp"
#include "../../imgui/imgui.h"
#include "../../imgui_sdl2.hpp"
#include "../entry_field.hpp"
#include "../child.hpp"

namespace rev {
	namespace {
		const char		*lb_openglId = "OpenGL Id",
						*lb_linearMag = "Linear-Mag",
						*lb_linearMin = "Linear-Min",
						*lb_wrapS = "Wrap-S",
						*lb_wrapT = "Wrap-T",
						*lb_mipState = "MipState",
						*lb_isCube = "Cube",
						*lb_face = "CubeFace",
						*lb_anisoCoeff = "Aniso-Coeff",
						*lb_size = "Size",
						*lb_format = "Format";
		const char		*lb_uri = "uri";
	}
	bool IGLTexture::guiEditor(const bool edit) {
		auto field = debug::EntryField("IGLTexture", edit);
		field.show(lb_openglId, _idTex);
		{
			const auto p0 = debug::MakeEditProxy<bool>(_iLinearMag);
			const auto p1 = debug::MakeEditProxy<bool>(_iLinearMin);
			field.entry(lb_linearMag, p0);
			field.entry(lb_linearMin, p1);
		}
		field.entry(lb_anisoCoeff, _coeff);
		if(field.entry(lb_wrapS, _wrapS) | field.entry(lb_wrapT, _wrapT))
			setUVWrap(_wrapS, _wrapT);
		field.show(lb_mipState, _mipLevel);
		field.show(lb_size, _size);
		field.show(lb_isCube, _texFlag != GL_TEXTURE_2D);
		field.show(
			lb_face,
			(_faceFlag == GL_TEXTURE_2D) ?
			-1 : int(_faceFlag - GL_TEXTURE_CUBE_MAP_POSITIVE_X)
		);
		if(_format)
			field.show(lb_format, GLFormat::QueryEnumString(*_format));
		else
			field.show(lb_format, "(none)");

		ImGui::Columns(1);
		if(const auto c = debug::ChildPush("texture", 0, true, ImGuiWindowFlags_HorizontalScrollbar)) {
			ImGui::Image(
				mgr_gui.storeResource(shared_from_this()),
				ImVec2(_size.width, _size.height)
			);
		}
		return field.modified();
	}
	bool Texture_URI::guiEditor(const bool edit) {
		auto field = debug::EntryField(getDebugName(), edit);
		field.entry(lb_uri, _uri);
		ImGui::Columns(1);
		return field.modified() | IGLTexture::guiEditor(edit);
	}
}
