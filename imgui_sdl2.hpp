#pragma once
#include "handle.hpp"
#include "spine/singleton.hpp"
#include "clock.hpp"
#include "gl_header.hpp"
#include <list>

struct ImDrawData;
using ImTextureID = void*;
namespace rev {
	class Window;
	using SPWindow = std::shared_ptr<Window>;
	class VDecl;
	using VDecl_SP = std::shared_ptr<VDecl>;
	struct ITech;
	using Tech_SP = std::shared_ptr<ITech>;

	#define mgr_gui (::rev::ImGui_SDL2::ref())
	class ImGui_SDL2 :
		public spi::Singleton<ImGui_SDL2>
	{
		private:
			HTex		_font;
			HInput		_keyboard,
						_mouse;
			Tech_SP		_tech;
			HFx			_effect;
			struct {
				GLint	texture,
						projMat;
			} _unif;
			VDecl_SP	_vdecl;

			using Void_SPC = std::shared_ptr<const void>;
			using ResV = std::list<Void_SPC>;
			ResV		_resV;

			void _clearResource();
			void _initFontsTexture();
			void _initTech();
			void _renderDrawLists(ImDrawData* draw_data);
		public:	
			ImGui_SDL2(const HInput& keyboard, const HInput& mouse, const Window& w);
			~ImGui_SDL2();
			void newFrame(const HFx& fx, const Window& window, Duration delta);
			void endFrame();
			ImTextureID storeResource(const Void_SPC& r);
	};
}
