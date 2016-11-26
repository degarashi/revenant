#pragma once
#include <SDL_video.h>
#include <memory>
#include "lubee/size.hpp"
#include "spine/enum.hpp"

namespace rev {
	class Window;
	using SPWindow = std::shared_ptr<Window>;
	class Window {
		public:
			enum class Stat {
				Minimized,
				Maximized,
				Hidden,
				Fullscreen,
				Shown
			};
			DefineEnumPair(
				Profile,
				((None)(0))
				((Compatibility)(SDL_GL_CONTEXT_PROFILE_COMPATIBILITY))
				((Core)(SDL_GL_CONTEXT_PROFILE_CORE))
				((ES)(SDL_GL_CONTEXT_PROFILE_ES))
			);

			//! OpenGL初期化パラメータ
			struct GLParam {
				//! OpenGLバージョン(メジャー & マイナー)
				int			profile,
							verMajor,
							verMinor;
				//! 色深度(RGB + Depth)
				int			red,
							green,
							blue,
							depth,
							stencil;
				//! ダブルバッファフラグ(0 or 1)
				int			doublebuffer;

				GLParam() noexcept;
				void setStdAttributes() const noexcept;
				void getStdAttributes() noexcept;
			};
			//! Window初期化パラメータ
			struct Param {
				std::string		title;
				int				posx,
								posy;
				lubee::SizeI	size;
				uint32_t		flag;					//!< その他のフラグ

				Param() noexcept;
			};
		private:
			SDL_Window*	_window;
			Stat		_stat;
			Window(SDL_Window* w) noexcept;

			void _checkState() noexcept;
		public:
			static void SetGLAttributes() {}
			// SDL_GLパラメータ設定(可変引数バージョン)
			template <class... Ts>
			static void SetGLAttributes(SDL_GLattr attr, int value, Ts... ts) {
				SDL_GL_SetAttribute(attr, value);
				SetGLAttributes(ts...);
			}
			static void GetGLAttributes() {}
			// SDL_GLパラメータ取得(可変引数バージョン)
			template <class... Ts>
			static void GetGLAttributes(SDL_GLattr attr, int& dst, Ts&&... ts) {
				SDL_GL_GetAttribute(attr, &dst);
				GetGLAttributes(std::forward<Ts>(ts)...);
			}

			static SPWindow Create(const Param& p);
			static SPWindow Create(const std::string& title, int w, int h, uint32_t flag=0);
			static SPWindow Create(const std::string& title, int x, int y, int w, int h, uint32_t flag=0);
			~Window();

			void setFullscreen(bool bFull) noexcept;
			void setGrab(bool bGrab) noexcept;
			void setMaximumSize(int w, int h) noexcept;
			void setMinimumSize(int w, int h) noexcept;
			void setSize(int w, int h) noexcept;
			void setTitle(const std::string& title) noexcept;
			void show(bool bShow) noexcept;
			void maximize() noexcept;
			void minimize() noexcept;
			void restore() noexcept;
			void setPosition(int x, int y) noexcept;
			void raise() noexcept;
			// for logging
			uint32_t getID() const noexcept;

			Stat getState() const noexcept;
			bool isGrabbed() const noexcept;
			bool isResizable() const noexcept;
			bool hasInputFocus() const noexcept;
			bool hasMouseFocus() const noexcept;
			lubee::SizeI getSize() const noexcept;
			lubee::SizeI getMaximumSize() const noexcept;
			lubee::SizeI getMinimumSize() const noexcept;
			uint32_t getSDLFlag() const noexcept;
			SDL_Window* getWindow() const noexcept;

			static void EnableScreenSaver(bool bEnable) noexcept;
	};
	std::ostream& operator << (std::ostream& os, const Window::GLParam& p);
	std::ostream& operator << (std::ostream& os, const Window::Param& p);
}
