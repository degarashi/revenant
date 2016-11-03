#pragma once
#include "drawtoken.hpp"
#include "../gl_header.hpp"

namespace rev {
	namespace draw {
		template <class T>
		struct Uniform : TokenT<T> {
			GLint	idUnif;
			Uniform(const GLint id):
				idUnif(id)
			{}
		};
	}
}
