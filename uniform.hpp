#pragma once
#include "glx_if.hpp"

namespace rev {
	struct IUniform {
		virtual void apply(Prog_Unif& pu, const GLint id) const = 0;
	};
	using Uniform_UP = std::unique_ptr<IUniform>;

	// 任意の値の固定配列を持ち、Prog_Unifを渡すとそれをUniform(Vector)変数としてセット出来るクラス
	template <class T, int NMax>
	struct UniformArray : IUniform {
		std::array<T, NMax>		value;
		int						nvalue;
		void apply(Prog_Unif& pu, const GLint id) const override {
			pu.setUniform(id, value.data(), nvalue, true);
		}
	};
}
