#include "../../gl_texture.hpp"
#include "../constant.hpp"
#include "../../imgui/imgui.h"
#include "../../imgui_sdl2.hpp"
#include "../column.hpp"
#include "../entry.hpp"
#include "../group.hpp"
#include "../entry_field.hpp"

namespace rev {
	namespace debug {
		namespace inner {
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
			bool _Edit(::rev::IGLTexture& t) {
				bool ret = false;
				if(auto field = EntryField("IGLTexture", 12, 0, 2)) {
					const GroupPush group;
					field.edit(lb_openglId, t._idTex);
					{
						const auto p0 = MakeEditProxy<bool>(t._iLinearMag);
						const auto p1 = MakeEditProxy<bool>(t._iLinearMin);
						field.edit(lb_linearMag, p0);
						field.edit(lb_linearMin, p1);
					}
					field.edit(lb_anisoCoeff, t._coeff);
					if(field.edit(lb_wrapS, t._wrapS) | field.edit(lb_wrapT, t._wrapT))
						t.setUVWrap(t._wrapS, t._wrapT);
					field.show(lb_mipState, t._mipLevel);
					field.show(lb_size, t._size);
					field.show(lb_isCube, t._texFlag != GL_TEXTURE_2D);
					field.show(
						lb_face,
						(t._faceFlag == GL_TEXTURE_2D) ?
						-1 : int(t._faceFlag - GL_TEXTURE_CUBE_MAP_POSITIVE_X)
					);
					if(t._format)
						field.show(lb_format, GLFormat::QueryEnumString(*t._format));
					else
						field.show(lb_format, "(none)");
					ret |= field.modified();
				}
				ImGui::Image(
					mgr_gui.storeResource(t.shared_from_this()),
					ImVec2(t._size.width, t._size.height)
				);
				return ret;
			}
			bool _Edit(::rev::Texture_URI& t) {
				if(auto field = EntryField("Texture_URI", 1, 0, 2)) {
					field.show(lb_uri, t._uri);
				}
				return _Edit(static_cast<::rev::IGLTexture&>(t));
			}
		}
	}
}
