#include "resmgr_app.hpp"
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
#include "u_matrix2d.hpp"
#include "fbrect.hpp"
#include "primitive.hpp"
#include <SDL_version.h>

namespace {
	const char* GetClipboardText(void*) {
		return SDL_GetClipboardText();
	}
	void SetClipboardText(void*, const char* text) {
		SDL_SetClipboardText(text);
	}
}
namespace rev {
	// --------------- ImGui_SDL2::Store ---------------
	ImGui_SDL2::Store::Store() {
		clear();
	}
	void ImGui_SDL2::Store::clear() {
		idToHdl.clear();
		ptrToId.clear();
	}
	uintptr_t ImGui_SDL2::Store::add(const HTexC& t, const uintptr_t id) {
		D_Assert0(idToHdl.size() == ptrToId.size());
		const auto b0 = idToHdl.emplace(id, TexF{t}).second,
					b1 = ptrToId.emplace(t.get(), id).second;
		D_Assert0(b0 && b1);
		return id;
	}
	HTexC ImGui_SDL2::_getTexture(const uintptr_t id) const {
		if(id == cs_fontId)
			return _font;
		const auto& i2h = _currentStore().idToHdl;
		const auto itr = i2h.find(id);
		D_Assert0(itr != i2h.end());
		return itr->second.handle;
	}

	// --------------- ImGui_SDL2 ---------------
	namespace {
		const FWVDecl g_vdecl(
			VDecl{
				{0,0, GL_FLOAT, GL_FALSE, 2, {VSemEnum::POSITION, 0}},
				{0,8, GL_FLOAT, GL_FALSE, 2, {VSemEnum::TEXCOORD, 0}},
				{0,16, GL_UNSIGNED_BYTE, GL_TRUE, 4, {VSemEnum::COLOR, 0}}
			}
		);
	}
	const uintptr_t ImGui_SDL2::cs_invalidId = std::numeric_limits<uintptr_t>::max(),
					ImGui_SDL2::cs_fontId = cs_invalidId-1;
	ImGui_SDL2::ImGui_SDL2(const HInput& keyboard, const HInput& mouse, const Window& window):
		_keyboard(keyboard),
		_mouse(mouse),
		_idStack(cs_fontId, 0),
		_storeSw(0),
		_pointerOnGUI(false)
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
		ImGui::GetIO().Fonts->TexID = reinterpret_cast<ImTextureID>(cs_invalidId);
		ImGui::DestroyContext();
	}
	ImTextureID ImGui_SDL2::storeTexture(const HTexC& t) {
		auto& cur = _currentStore();
		auto& prev = _prevStore();

		if(const auto itr = cur.ptrToId.find(t.get());
			itr != cur.ptrToId.end())
		{
			// 今のフレームで使用された
			return reinterpret_cast<ImTextureID>(itr->second);
		}
		if(const auto itr = prev.ptrToId.find(t.get());
			itr != prev.ptrToId.end())
		{
			// 前のフレームで使用された
			prev.idToHdl.at(itr->second).used = true;
			return reinterpret_cast<ImTextureID>(cur.add(t, itr->second));
		}
		// 新しくIDを生成
		return reinterpret_cast<ImTextureID>(cur.add(t, _idStack.get()));
	}
	const ImGui_SDL2::Store& ImGui_SDL2::_prevStore() const {
		return _store[_storeSw^1];
	}
	ImGui_SDL2::Store& ImGui_SDL2::_prevStore() {
		return _store[_storeSw^1];
	}
	const ImGui_SDL2::Store& ImGui_SDL2::_currentStore() const {
		return _store[_storeSw];
	}
	ImGui_SDL2::Store& ImGui_SDL2::_currentStore() {
		return _store[_storeSw];
	}

	void ImGui_SDL2::_switchResource() {
		{
			// 古いIDを回収
			const auto& prev = _prevStore();
			for(auto& p : prev.idToHdl) {
				if(!p.second.used) {
					_idStack.put(p.first);
				}
			}
		}
		_storeSw ^= 1;
		_currentStore().clear();
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

		for(int i=0 ; i<3 ; i++)
			io.MouseDown[i] = static_cast<bool>(_mouse->getButton(i));
		if(_mouse->getMouseMode() == MouseMode::Absolute) {
			const frea::Vec2 mpos = _mouse->getPointer().lock()->absPos;
			io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
			{
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
		}
		// Start the frame. This call will update the io.WantCaptureMouse, io.WantCaptureKeyboard flag that you can use to dispatch inputs (or not) to your application.
		ImGui::NewFrame();
	}
	void ImGui_SDL2::endFrame() {
		ImGui::Render();
		_switchResource();
		_pointerOnGUI = ImGui::IsAnyWindowHovered() | ImGui::IsAnyItemHovered();
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
		io.Fonts->TexID = reinterpret_cast<ImTextureID>(cs_fontId);
	}
	void ImGui_SDL2::_initTech() {
		auto tp = mgr_gl.loadTechPass("imgui.glx");
		_tech = tp->getTechnique("ImGui|Default");
		auto& p = *_tech->getProgram();
		_unif.texture = *p.getUniformId(SName("Texture"));
		_unif.projMat = *p.getUniformId(SName("ProjMtx"));
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
		u.setUniformById(_unif.projMat, ortho_projection);
		dynamic_cast<U_Matrix2D&>(e).setWorld(frea::Mat3::Identity());
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
					u.setUniformById(_unif.texture, _getTexture(reinterpret_cast<uintptr_t>(pcmd->TextureId)));
					e.setScissor({true, {
						(int)pcmd->ClipRect.x,
						(int)pcmd->ClipRect.z,
						(int)(fb_height - pcmd->ClipRect.w),
						(int)(-pcmd->ClipRect.y + fb_height)
					}});
					const auto prim = std::make_shared<Primitive>();
					prim->vdecl = g_vdecl;
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
	bool ImGui_SDL2::pointerOnGUI() const noexcept {
		return _pointerOnGUI;
	}
}
