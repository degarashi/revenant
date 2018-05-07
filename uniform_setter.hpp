#pragma once
#include <vector>
#include <functional>

namespace rev {
	class UniformEnt;
	class GLProgram;
	using UniformSetF = std::function<void (const void*, UniformEnt&)>;
	using UniformSetF_V = std::vector<UniformSetF>;
	struct IUniformSetter {
		virtual ~IUniformSetter() {}
		virtual void applyUniform(UniformEnt&, const GLProgram&) const {}
		virtual UniformSetF getUniformF(const GLProgram&) const { return nullptr; }
	};
}
