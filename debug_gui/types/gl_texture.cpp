#include "../../gl_texture.hpp"
#include "../../imgui/imgui.h"
#include "../../imgui_sdl2.hpp"
#include "../entry_field.hpp"

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
	bool IGLTexture::guiEditor(bool) {
		bool ret = false;
		if(auto field = debug::EntryField("IGLTexture")) {
			field.show(lb_openglId, _idTex);
			{
				const auto p0 = debug::MakeEditProxy<bool>(_iLinearMag);
				const auto p1 = debug::MakeEditProxy<bool>(_iLinearMin);
				field.edit(lb_linearMag, p0);
				field.edit(lb_linearMin, p1);
			}
			field.edit(lb_anisoCoeff, _coeff);
			if(field.edit(lb_wrapS, _wrapS) | field.edit(lb_wrapT, _wrapT))
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
			ret |= field.modified();

			ImGui::Columns(1);
			ImGui::Image(
				mgr_gui.storeResource(shared_from_this()),
				ImVec2(_size.width, _size.height)
			);
		}
		return ret;
	}
	bool Texture_URI::guiEditor(bool) {
		if(auto field = debug::EntryField(getDebugName())) {
			field.edit(lb_uri, _uri);
			ImGui::Columns(1);
			return IGLTexture::guiEditor();
		}
		return false;
	}
}
