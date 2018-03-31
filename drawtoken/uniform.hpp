#pragma once
#include "token.hpp"

namespace rev::draw {
	struct IUniform {
		virtual ~IUniform() {}
		virtual bool isArray() const noexcept = 0;
	};
	template <class T>
	struct Uniform :
		TokenT<T>,
		IUniform
	{
		mutable GLint	idUnif=-1;
		void exportToken(TokenDst& dst, const GLint id, const int /*activeTexId*/ = -1) const override {
			idUnif = id;
			this->clone(dst);
		}
	};
}
