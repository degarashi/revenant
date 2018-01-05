#pragma once
#include "token.hpp"

namespace rev {
	namespace draw {
		template <class T>
		struct Uniform : TokenT<T> {
			mutable GLint	idUnif=-1;
			void exportToken(TokenDst& dst, const GLint id, const int /*activeTexId*/ = -1) const override {
				idUnif = id;
				this->clone(dst);
			}
		};
	}
}
