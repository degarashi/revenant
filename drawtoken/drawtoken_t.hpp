#pragma once
#include "token.hpp"
#include "../gl_header.hpp"
#include "lubee/error.hpp"

namespace rev {
	namespace draw {
		template <class T>
		struct Uniform : TokenT<T> {
			GLint	idUnif;
			Uniform(const GLint id):
				idUnif(id)
			{
				// 値が決め打ちだが、これ以上の範囲は恐らくバグってるだろうとの想定
				D_Assert0(id < 0x10000);
			}
		};
	}
}
