#pragma once
#include "handle/opengl.hpp"
#include "handle/input.hpp"
#include "spine/singleton.hpp"
#include "clock.hpp"
#include "gl_header.hpp"
#include "lubee/freelist.hpp"

struct ImDrawData;
using ImTextureID = void*;
namespace rev {
	class Window;
	#define mgr_gui (::rev::ImGui_SDL2::ref())
	class ImGui_SDL2 :
		public spi::Singleton<ImGui_SDL2>
	{
		private:
			const static uintptr_t		cs_fontId,
										cs_invalidId;
			HTex		_font;
			HInput		_keyboard,
						_mouse;
			HTech		_tech;
			HFx			_effect;
			struct {
				GLint	texture,
						projMat;
			} _unif;
			FWVDecl		_vdecl;

			lubee::Freelist<uintptr_t>	_idStack;
			struct Store {
				struct TexF {
					HTexC	handle;
					bool	used;

					TexF(const HTexC& t):
						handle(t),
						used(false)
					{}
				};
				using TexIdToHdl = std::unordered_map<uintptr_t, TexF>;
				using TexPtrToId = std::unordered_map<const GLTexture*, uintptr_t>;
				TexIdToHdl	idToHdl;
				TexPtrToId	ptrToId;

				Store();
				uintptr_t add(const HTexC& t, uintptr_t id);
				void clear();
			} _store[2];
			bool			_storeSw;
			const Store& _prevStore() const;
			const Store& _currentStore() const;
			Store& _prevStore();
			Store& _currentStore();

			bool _pointerOnGUI;

			HTexC _getTexture(uintptr_t id) const;
			void _switchResource();
			void _initFontsTexture();
			void _initTech();
			void _renderDrawLists(ImDrawData* draw_data);
		public:
			ImGui_SDL2(const HInput& keyboard, const HInput& mouse, const Window& w);
			~ImGui_SDL2();
			void newFrame(const HFx& fx, const Window& window, Duration delta);
			void endFrame();
			ImTextureID storeTexture(const HTexC& t);
			bool pointerOnGUI() const noexcept;
	};
}
