#pragma once
#include <memory>

namespace rev {
	// メモリの都合上で外部メモリにクラス情報を置きたい場合に使用するラッパークラス
	template <class T>
	struct UPObj {
		using Pointer = std::unique_ptr<T>;
		Pointer pointer;

		UPObj():
			pointer(new T())
		{}
		UPObj(const T& t):
			pointer(new T(t))
		{}
		UPObj(T&& t):
			pointer(new T(std::move(t)))
		{}
		UPObj(const UPObj& obj):
			pointer(new T(*obj.pointer))
		{}
		UPObj(UPObj&& obj) noexcept:
			pointer(std::move(obj.pointer))
		{}
		UPObj& operator = (const UPObj& obj) {
			*pointer = *obj.pointer;
			return *this;
		}
		UPObj& operator = (UPObj& obj) noexcept {
			*pointer = std::move(*obj.pointer);
			return *this;
		}
		T& operator * () noexcept {
			return *pointer;
		}
		const T& operator * () const noexcept {
			return *pointer;
		}
		bool operator == (const UPObj& obj) const noexcept {
			return *pointer == *obj.pointer;
		}
		bool operator != (const UPObj& obj) const noexcept {
			return !(this->operator == (obj));
		}
	};
}

#include <functional>
namespace std {
	template <class T>
	struct hash<rev::UPObj<T>> {
		std::size_t operator()(const rev::UPObj<T>& t) const noexcept {
			return std::hash<T>()(*t);
		}
	};
}
