#include "imgui_sdl2.hpp"
#include "sdl_window.hpp"
#include "imgui/imgui.h"
#include "input.hpp"
#include "gl_resource.hpp"
#include "gl_texture.hpp"
#include "gl_shader.hpp"
#include "gl_program.hpp"
#include "vdecl.hpp"
#include <SDL_clipboard.h>
#include "tech_pass.hpp"
#include "tech_if.hpp"
#include "glx_if.hpp"
#include "uniform_ent.hpp"
#include "drawtoken/make_uniform.hpp"
#include "sys_uniform.hpp"
#include "fbrect.hpp"
#include "primitive.hpp"

namespace {
	const char* GetClipboardText(void*) {
		return SDL_GetClipboardText();
	}
	void SetClipboardText(void*, const char* text) {
		SDL_SetClipboardText(text);
	}
}
namespace rev {
	namespace vdecl {
		struct imgui {};
	}
	DefineVDecl(vdecl::imgui)
	const VDecl_SP& DrawDecl<vdecl::imgui>::GetVDecl() {
		static VDecl_SP vd(new VDecl{
			{0,0, GL_FLOAT, GL_FALSE, 2, {VSem::POSITION, 0}},
			{0,8, GL_FLOAT, GL_FALSE, 2, {VSem::TEXCOORD, 0}},
			{0,16, GL_UNSIGNED_BYTE, GL_TRUE, 4, {VSem::COLOR, 0}}
		});
		return vd;
	}
	ImGui_SDL2::ImGui_SDL2(const HInput& keyboard, const HInput& mouse, const Window& window):
		_keyboard(keyboard),
		_mouse(mouse)
	{
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.KeyMap[ImGuiKey_Tab] = SDLK_TAB;
		io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
		io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
		io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
		io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
		io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
		io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
		io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
		io.KeyMap[ImGuiKey_Insert] = SDL_SCANCODE_INSERT;
		io.KeyMap[ImGuiKey_Delete] = SDLK_DELETE;
		io.KeyMap[ImGuiKey_Backspace] = SDLK_BACKSPACE;
		io.KeyMap[ImGuiKey_Enter] = SDLK_RETURN;
		io.KeyMap[ImGuiKey_Escape] = SDLK_ESCAPE;
		io.KeyMap[ImGuiKey_Space] = SDLK_SPACE;
		io.KeyMap[ImGuiKey_A] = SDLK_a;
		io.KeyMap[ImGuiKey_C] = SDLK_c;
		io.KeyMap[ImGuiKey_V] = SDLK_v;
		io.KeyMap[ImGuiKey_X] = SDLK_x;
		io.KeyMap[ImGuiKey_Y] = SDLK_y;
		io.KeyMap[ImGuiKey_Z] = SDLK_z;

		io.RenderDrawListsFn = [](ImDrawData* d) { mgr_gui._renderDrawLists(d); };
		io.SetClipboardTextFn = &SetClipboardText;
		io.GetClipboardTextFn = &GetClipboardText;
		io.ClipboardUserData = nullptr;
		#ifdef _WIN32
			SDL_SysWMinfo wmInfo;
			SDL_VERSION(&wmInfo.version);
			SDL_GetWindowWMInfo(window.getWindow(), &wmInfo);
			io.ImeWindowHandle = wmInfo.info.win.window;
		#else
			(void)window;
		#endif
		_initFontsTexture();
		_initTech();
	}
	ImGui_SDL2::~ImGui_SDL2() {
		ImGui::GetIO().Fonts->TexID = 0;
		ImGui::DestroyContext();
	}
	void ImGui_SDL2::newFrame(const HFx& fx, const Window& window, const Duration delta) {
		_effect = fx;
		ImGuiIO& io = ImGui::GetIO();
		// Setup display size (every frame to accommodate for window resizing)
		{
			const auto ws = window.getSize().toSize<float>();
			const auto disp = window.getDrawableSize().toSize<float>();
			io.DisplaySize = ImVec2{ws.width, ws.height};
			io.DisplayFramebufferScale = ImVec2 {
				ws.width > 0 ? (disp.width / ws.width) : 0,
				ws.height > 0 ? (disp.height / ws.height) : 0
			};
		}
		{
			// Setup time step
			const uint64_t delta_ms = std::chrono::duration_cast<Microseconds>(delta).count();
			io.DeltaTime = delta_ms / 1000'000.f;
		}
		// Setup mouse inputs (we already got mouse wheel, keyboard keys & characters from our event handler)
		io.MouseWheelH = _mouse->getAxis(2);
		io.MouseWheel = _mouse->getAxis(3);
		{
			const auto& s = mgr_input.getTextInput();
			if(!s.empty())
				io.AddInputCharactersUTF8(s.c_str());
		}
		{
			const auto& log = mgr_input.getKeyLog();
			for(auto& k : log)
				io.KeysDown[k.ch] = k.down;
		}
		{
			const auto& aux = mgr_input.getKeyAux();
			io.KeyShift = aux.shift;
			io.KeyCtrl = aux.ctrl;
			io.KeyAlt = aux.alt;
			io.KeySuper = aux.super;
		}

		const frea::Vec2 mpos = _mouse->getPointer().lock()->absPos;
		for(int i=0 ; i<3 ; i++)
			io.MouseDown[i] = static_cast<bool>(_mouse->getButton(i));
		{
			io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
			// We need to use SDL_CaptureMouse() to easily retrieve mouse coordinates outside of the client area. This is only supported from SDL 2.0.4 (released Jan 2016)
			const auto wflag = window.getSDLFlag();
			#if (SDL_MAJOR_VERSION >= 2) && (SDL_MINOR_VERSION >= 0) && (SDL_PATCHLEVEL >= 4)
				if (wflag & (SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_MOUSE_CAPTURE))
					io.MousePos = ImVec2(mpos.x, mpos.y);
				bool any_mouse_button_down = false;
				for (int n = 0; n < IM_ARRAYSIZE(io.MouseDown); n++)
					any_mouse_button_down |= io.MouseDown[n];
				if (any_mouse_button_down && !(wflag & SDL_WINDOW_MOUSE_CAPTURE))
					SDL_CaptureMouse(SDL_TRUE);
				if (!any_mouse_button_down && (wflag & SDL_WINDOW_MOUSE_CAPTURE))
					SDL_CaptureMouse(SDL_FALSE);
			#else
				if ((wflag & SDL_WINDOW_INPUT_FOCUS) != 0)
					io.MousePos = ImVec2(mpos.x, mpos.y);
			#endif
		}
		// Hide OS mouse cursor if ImGui is drawing it
		SDL_ShowCursor(io.MouseDrawCursor ? SDL_DISABLE : SDL_ENABLE);
		// Start the frame. This call will update the io.WantCaptureMouse, io.WantCaptureKeyboard flag that you can use to dispatch inputs (or not) to your application.
		ImGui::NewFrame();
	}
	void ImGui_SDL2::endFrame() {
		ImGui::Render();
	}
	void ImGui_SDL2::_initFontsTexture() {
		// Build texture atlas
		ImGuiIO& io = ImGui::GetIO();
		uint8_t* pixels;
		lubee::SizeI size;
		// Load as RGBA 32-bits for OpenGL3 demo because it is more likely to be compatible with user's existing shader.
		io.Fonts->GetTexDataAsRGBA32(&pixels, &size.width, &size.height);

		// Upload texture to graphics system
		_font = mgr_gl.createTextureInit(
			size,
			GL_RGBA,
			false,
			true,
			GL_UNSIGNED_BYTE,
			AB_Byte(pixels, size.width*size.height*4)
		);
		_font->setFilter(true, true);
		// Store our identifier
		io.Fonts->TexID = reinterpret_cast<ImTextureID>(&_font);
	}
	void ImGui_SDL2::_initTech() {
		auto tp = mgr_gl.loadTechPass("imgui.glx");
		_tech = tp->getTechnique("ImGui|Default");
		auto& p = *_tech->getProgram();
		_unif.texture = *p.getUniformId("Texture");
		_unif.projMat = *p.getUniformId("ProjMtx");
	}
	void ImGui_SDL2::_renderDrawLists(ImDrawData* draw_data) {
		ImGuiIO& io = ImGui::GetIO();
		const int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
		const int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
		if (fb_width == 0 || fb_height == 0)
			return;
		draw_data->ScaleClipRects(io.DisplayFramebufferScale);

		auto& e = *_effect;
		const auto prev_vp = e.setViewport({true, {0, fb_width, 0, fb_height}});
		const frea::Mat4 ortho_projection(
			 2.0f/io.DisplaySize.x, 0.0f,                   0.0f, 0.0f,
			 0.0f,                  2.0f/-io.DisplaySize.y, 0.0f, 0.0f,
			 0.0f,                  0.0f,                  -1.0f, 0.0f,
			-1.0f,                  1.0f,                   0.0f, 1.0f
		);
		const auto prev_tech = e.setTechnique(_tech);
		auto& u = e.refUniformEnt();
		const auto prev_sci = e.setScissor({false, {0,1,0,1}});
		u.setUniform(_unif.projMat, draw::MakeUniform(ortho_projection));
		e.ref2D().setWorld(frea::Mat3::Identity());
		for (int n=0; n < draw_data->CmdListsCount; n++) {
			const ImDrawList* cmd_list = draw_data->CmdLists[n];
			const ImDrawIdx* idx_buffer_offset = 0;

			const auto vb = mgr_gl.makeVBuffer(DrawType::Static);
			vb->initData((const GLvoid*)cmd_list->VtxBuffer.Data, (GLsizeiptr)cmd_list->VtxBuffer.Size, sizeof(ImDrawVert));

			const auto ib = mgr_gl.makeIBuffer(DrawType::Static);
			ib->initData((const GLvoid*)cmd_list->IdxBuffer.Data, (GLsizeiptr)cmd_list->IdxBuffer.Size, sizeof(ImDrawIdx));

			for (int cmd_i=0; cmd_i < cmd_list->CmdBuffer.Size ; cmd_i++) {
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
				if (pcmd->UserCallback) {
					pcmd->UserCallback(cmd_list, pcmd);
				}
				else {
					u.setUniform(_unif.texture, draw::MakeUniform(*reinterpret_cast<HTex*>(pcmd->TextureId)));
					e.setScissor({true, {
						(int)pcmd->ClipRect.x,
						(int)pcmd->ClipRect.z,
						(int)(fb_height - pcmd->ClipRect.w),
						(int)(-pcmd->ClipRect.y + fb_height)
					}});
					const auto prim = std::make_shared<Primitive>();
					prim->vdecl = DrawDecl<vdecl::imgui>::GetVDecl();
					prim->drawMode = DrawMode::Triangles;
					prim->ib = ib;
					prim->vb[0] = vb;
					auto& wi = prim->withIndex;
					wi.count = (GLsizei)pcmd->ElemCount;
					wi.offsetElem = uintptr_t(idx_buffer_offset) / sizeof(ImDrawIdx);
					D_Assert0(uintptr_t(idx_buffer_offset) % sizeof(ImDrawIdx) == 0);
					e.setPrimitive(prim);
					e.draw();
				}
				idx_buffer_offset += pcmd->ElemCount;
			}
		}
		e.setScissor(prev_sci);
		if(prev_tech)
			e.setTechnique(prev_tech);
		e.setViewport(prev_vp);
	}
}