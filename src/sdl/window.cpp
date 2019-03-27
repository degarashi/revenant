#include "window.hpp"
#include "error.hpp"
#include "lubee/src/meta/countof.hpp"

namespace rev {
	// --------------------- Param ---------------------
	Window::Param::Param() noexcept {
		posx = posy = SDL_WINDOWPOS_UNDEFINED;
		size.width = 640;
		size.height = 480;
		flag = SDL_WINDOW_SHOWN;
	}
	std::ostream& operator << (std::ostream& os, const Window::Param& p) {
		os << "WindowParam: ["
			<< "posx=" << p.posx
			<< ", posy=" << p.posy
			<< ", size=" << p.size
			<< ", flag=" << p.flag
			<< "]";
		return os;
	}
	Window::GLParam::GLParam() noexcept {
		profile = Profile::None;
		verMajor = 2;
		verMinor = 0;
		red = green = blue = 5;
		depth = 16;
		doublebuffer = 1;
		stencil = 0;
	}
	namespace {
		const std::string c_profileStr[Window::Profile::_Num] = {
			"None",
			"Compatibility",
			"Core",
			"ES"
		};
		const std::string& ProfileString(const int idx) noexcept {
			D_Assert0(idx < Window::Profile::_Num);
			return c_profileStr[idx];
		}
	}
	std::ostream& operator << (std::ostream& os, const Window::GLParam& p) {
		os << "WindowGLParam: ["
			<< "profile=" << ProfileString(p.profile)
			<< ", verMajor=" << p.verMajor
			<< ", verMinor=" << p.verMinor
			<< ", red=" << p.red
			<< ", green=" << p.green
			<< ", blue=" << p.blue
			<< ", depth=" << p.depth
			<< ", stencil=" << p.stencil
			<< ", doublebuffer=" << p.doublebuffer
			<< "]";
		return os;
	}
	namespace {
		const SDL_GLattr c_AttrId[] = {
			SDL_GL_CONTEXT_PROFILE_MASK,
			SDL_GL_CONTEXT_MAJOR_VERSION,
			SDL_GL_CONTEXT_MINOR_VERSION,
			SDL_GL_RED_SIZE,
			SDL_GL_GREEN_SIZE,
			SDL_GL_BLUE_SIZE,
			SDL_GL_DEPTH_SIZE,
			SDL_GL_STENCIL_SIZE,
			SDL_GL_DOUBLEBUFFER
		};
		using IPtr = int (Window::GLParam::*);
		const IPtr c_AttrPtr[] = {
			&Window::GLParam::profile,
			&Window::GLParam::verMajor,
			&Window::GLParam::verMinor,
			&Window::GLParam::red,
			&Window::GLParam::green,
			&Window::GLParam::blue,
			&Window::GLParam::depth,
			&Window::GLParam::stencil,
			&Window::GLParam::doublebuffer
		};
	}
	void Window::GLParam::getStdAttributes() noexcept {
		for(int i=0 ; i<static_cast<int>(countof(c_AttrId)) ; i++)
			Window::GetGLAttributes(c_AttrId[i], this->*c_AttrPtr[i]);
	}
	void Window::GLParam::setStdAttributes() const noexcept {
		for(int i=0 ; i<static_cast<int>(countof(c_AttrId)) ; i++)
			Window::SetGLAttributes(c_AttrId[i], this->*c_AttrPtr[i]);
	}
	// --------------------- Window ---------------------
	HWin Window::Create(const Param& p) {
		return Create(p.title, p.posx, p.posy, p.size.width, p.size.height, p.flag);
	}
	HWin Window::Create(const std::string& title, const int w, const int h, const uint32_t flag) {
		return Create(title, 128, 128, w, h, flag);
	}
	HWin Window::Create(const std::string& title, const int x, const int y, const int w, const int h, const uint32_t flag) {
		return HWin(new Window(SDLWarn(SDL_CreateWindow, title.c_str(), x, y, w, h, flag|SDL_WINDOW_OPENGL)));
	}
	Window::Window(SDL_Window* w) noexcept:
		_window(w)
	{
		_checkState();
	}
	Window::~Window() {
		D_SDLWarn(SDL_DestroyWindow, _window);
	}
	void Window::_checkState() noexcept {
		const uint32_t f = getSDLFlag();
		if(f & SDL_WINDOW_SHOWN)
			_stat = Stat::Hidden;
		else if(f & SDL_WINDOW_FULLSCREEN)
			_stat = Stat::Fullscreen;
		else if(f & SDL_WINDOW_MINIMIZED)
			_stat = Stat::Minimized;
		else if(f & SDL_WINDOW_MAXIMIZED)
			_stat = Stat::Maximized;
		else
			_stat = Stat::Shown;
	}
	void Window::setFullscreen(const bool bFull) noexcept {
		D_SDLWarn(SDL_SetWindowFullscreen, _window, bFull ? SDL_WINDOW_FULLSCREEN : 0);
		_checkState();
	}
	void Window::setGrab(const bool bGrab) noexcept {
		D_SDLWarn(SDL_SetWindowGrab, _window, bGrab ? SDL_TRUE : SDL_FALSE);
	}
	void Window::setMaximumSize(const int w, const int h) noexcept {
		D_SDLWarn(SDL_SetWindowMaximumSize, _window, w, h);
	}
	void Window::setMinimumSize(const int w, const int h) noexcept {
		D_SDLWarn(SDL_SetWindowMinimumSize, _window, w, h);
	}
	void Window::setSize(const int w, const int h) noexcept {
		D_SDLWarn(SDL_SetWindowSize, _window, w, h);
	}
	void Window::show(const bool bShow) noexcept {
		if(bShow)
			D_SDLWarn(SDL_ShowWindow, _window);
		else
			D_SDLWarn(SDL_HideWindow, _window);
		_checkState();
	}
	void Window::setTitle(const std::string& title) noexcept {
		D_SDLWarn(SDL_SetWindowTitle, _window, title.c_str());
	}
	void Window::maximize() noexcept {
		D_SDLWarn(SDL_MaximizeWindow, _window);
		_checkState();
	}
	void Window::minimize() noexcept {
		D_SDLWarn(SDL_MinimizeWindow, _window);
		_checkState();
	}
	void Window::restore() noexcept {
		D_SDLWarn(SDL_RestoreWindow, _window);
		_checkState();
	}
	void Window::setPosition(const int x, const int y) noexcept {
		D_SDLWarn(SDL_SetWindowPosition, _window, x, y);
	}
	void Window::raise() noexcept {
		D_SDLWarn(SDL_RaiseWindow, _window);
		_checkState();
	}
	uint32_t Window::getID() const noexcept {
		return D_SDLWarn(SDL_GetWindowID, _window);
	}
	Window::Stat Window::getState() const noexcept {
		return _stat;
	}
	bool Window::isGrabbed() const noexcept {
		return D_SDLWarn(SDL_GetWindowGrab, _window) == SDL_TRUE;
	}
	bool Window::isResizable() const noexcept {
		return getSDLFlag() & SDL_WINDOW_RESIZABLE;
	}
	bool Window::hasInputFocus() const noexcept {
		return getSDLFlag() & SDL_WINDOW_INPUT_FOCUS;
	}
	bool Window::hasMouseFocus() const noexcept {
		return getSDLFlag() & SDL_WINDOW_MOUSE_FOCUS;
	}
	namespace {
		auto GetSize(void (*func)(SDL_Window*,int*,int*), SDL_Window* wd) noexcept {
			int w,h;
			D_SDLWarn(func, wd, &w, &h);
			return lubee::SizeI(w,h);
		}
	}
	lubee::SizeI Window::getSize() const noexcept {
		return GetSize(SDL_GetWindowSize, _window);
	}
	lubee::SizeI Window::getMaximumSize() const noexcept {
		return GetSize(SDL_GetWindowMaximumSize, _window);
	}
	lubee::SizeI Window::getMinimumSize() const noexcept {
		return GetSize(SDL_GetWindowMinimumSize, _window);
	}
	lubee::SizeI Window::getDrawableSize() const noexcept {
		return GetSize(SDL_GL_GetDrawableSize, _window);
	}
	uint32_t Window::getSDLFlag() const noexcept {
		return D_SDLWarn(SDL_GetWindowFlags, _window);
	}
	SDL_Window* Window::getWindow() const noexcept {
		return _window;
	}
	void Window::EnableScreenSaver(const bool bEnable) noexcept {
		if(bEnable)
			D_SDLWarn(SDL_EnableScreenSaver);
		else
			D_SDLWarn(SDL_DisableScreenSaver);
	}
}
