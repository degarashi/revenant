#pragma once
#include "sdl_error.hpp"
#include <SDL_thread.h>
#include <string>

namespace rev {
	//! thread local storage (with SDL)
	template <class T>
	class TLS {
		private:
			SDL_TLSID	_tlsID;
			static void Dtor(void* p) {
				delete reinterpret_cast<T*>(p);
			}
			T* _getPtr() {
				return reinterpret_cast<T*>(SDL_TLSGet(_tlsID));
			}
			const T* _getPtr() const {
				return reinterpret_cast<const T*>(SDL_TLSGet(_tlsID));
			}

		public:
			TLS() {
				_tlsID = SDLAssert(SDL_TLSCreate);
			}
			template <class... Args>
			TLS(Args&&... args): TLS() {
				*this = T(std::forward<Args>(args)...);
			}
			template <class TA>
			TLS& operator = (TA&& t) {
				T* p = _getPtr();
				if(!p)
					D_SDLWarn(SDL_TLSSet, _tlsID, new T(std::forward<TA>(t)), Dtor);
				else
					*p = std::forward<TA>(t);
				return *this;
			}
			bool valid() const {
				return static_cast<bool>(*this);
			}
			T& operator * () {
				return *_getPtr();
			}
			const T& operator * () const {
				return *_getPtr();
			}
			T& get() {
				return this->operator*();
			}
			const T& get() const {
				return this->operator*();
			}
			explicit operator bool() const {
				return _getPtr() != nullptr;
			}
			bool initialized() const {
				return static_cast<bool>(*this);
			}
			void terminate() {
				D_SDLAssert(SDL_TLSSet, _tlsID, nullptr, nullptr);
			}
	};
	extern TLS<SDL_threadID>	tls_threadID;
	extern TLS<std::string>		tls_threadName;
}
