#pragma once
#include <utility>

namespace rev {
	//! delete演算子を呼ぶデリータ
	template <class T>
	struct Del_Delete {
		void operator()(T* p) const { delete p; }
	};
	//! 何もしないデリータ
	template <class T>
	struct Del_Nothing {
		void operator()(T* /*p*/) const {}
	};

	//! メモリの解放 / not解放をフラグで管理する簡易スマートポインタ
	template <class T, class DEL=Del_Delete<T>>
	class FlagPtr {
		private:
			bool	_bDelete;
			T*		_ptr;
		public:
			FlagPtr() noexcept:
				_bDelete(false),
				_ptr(nullptr)
			{}
			FlagPtr(FlagPtr&& p) noexcept:
				_bDelete(p._bDelete),
				_ptr(p._ptr)
			{
				p._bDelete = false;
				p._ptr = nullptr;
			}
			FlagPtr(T* p, const bool bDel) noexcept:
				_bDelete(bDel),
				_ptr(p)
			{}
			~FlagPtr() {
				if(_bDelete) {
					DEL del;
					del(_ptr);
				}
			}
			void reset(T* p, const bool bDel) noexcept {
				this->~FlagPtr();
				new(this) FlagPtr(p, bDel);
			}
			void reset() noexcept {
				reset(nullptr, false);
			}
			//! コピー禁止
			void operator = (const FlagPtr& p) const = delete;
			void operator = (FlagPtr&& p) noexcept {
				this->~FlagPtr();
				new(this) FlagPtr(std::move(p));
			}
			T* operator -> () noexcept {
				return _ptr;
			}
			const T* operator -> () const noexcept {
				return _ptr;
			}
			T* get() noexcept {
				return _ptr;
			}
			const T* get() const noexcept {
				return _ptr;
			}
			bool operator == (const FlagPtr& fp) const noexcept {
				return _ptr == fp._ptr;
			}
			bool operator == (const T* p) const noexcept {
				return _ptr == p;
			}
	};
}
