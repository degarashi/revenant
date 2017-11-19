#pragma once
#include "sdl_mutex.hpp"

namespace rev {
	namespace detail {
		struct CallUnlockR {
			template <class T>
			void operator()(T& t) const noexcept(noexcept(t._unlockR())) {
				return t._unlockR();
			}
		};
		struct CallUnlock {
			template <class T>
			void operator()(T& t) const noexcept(noexcept(t._unlock())) {
				return t._unlock();
			}
		};
		template <class SP, class T, class UnlockT>
		class SpinInner {
			private:
				SP&		_src;
				T*		_data;

			public:
				SpinInner(const SpinInner&) = delete;
				SpinInner& operator = (const SpinInner&) = delete;
				SpinInner(SpinInner&& n) noexcept:
					_src(n._src),
					_data(n._data)
				{
					n._data = nullptr;
				}
				SpinInner(SP& src, T* data) noexcept:
					_src(src),
					_data(data)
				{}
				~SpinInner() {
					unlock();
				}
				T& operator * () noexcept { return *_data; }
				T* operator -> () noexcept { return _data; }
				bool valid() const noexcept { return _data != nullptr; }
				explicit operator bool () const noexcept { return valid(); }
				void unlock() noexcept(noexcept(std::declval<UnlockT>()(_src))) {
					if(_data) {
						UnlockT()(_src);
						_data = nullptr;
					}
				}
				template <class T2>
				auto castAndMove() noexcept {
					SpinInner<SP, T2, UnlockT> ret(_src, reinterpret_cast<T2*>(_data));
					_data = nullptr;
					return ret;
				}
				template <class T2>
				auto castAndMoveDeRef() noexcept {
					SpinInner<SP, T2, UnlockT> ret(_src, reinterpret_cast<T2*>(*_data));
					_data = nullptr;
					return ret;
				}
		};
	}
}
