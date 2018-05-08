#pragma once
#include <cstdint>
#include <utility>
#include "../gl_header.hpp"

namespace rev {
	namespace draw {
		struct TokenDst {
			virtual ~TokenDst() {}
			virtual void* allocate_memory(std::size_t s, intptr_t ofs) = 0;
		};
		struct Token {
			virtual ~Token() {}
			virtual void exec() = 0;
			virtual void clone(TokenDst& dst) const = 0;
			virtual void exportToken(TokenDst& dst, const GLint id=-1, const int activeTexId=-1) const = 0;
			virtual std::size_t getSize() const = 0;
		};
		template <class From, class To>
		intptr_t CalcPointerOffset() {
			return reinterpret_cast<intptr_t>(static_cast<To*>(reinterpret_cast<From*>(1))) - 1;
		}
		template <class T>
		intptr_t CalcTokenOffset() {
			return CalcPointerOffset<T, Token>();
		}
		template <class T>
		struct TokenT : Token {
			// 引数のバッファへ中身をcopyする
			void clone(TokenDst& dst) const override {
				new(dst.allocate_memory(getSize(), CalcTokenOffset<T>())) T(static_cast<const T&>(*this));
			}
			void exportToken(TokenDst& dst, const GLint /*id*/ = -1, const int /*activeTexId*/ = -1) const override {
				clone(dst);
			}
			std::size_t getSize() const override {
				return sizeof(T);
			}
		};

	}
}
