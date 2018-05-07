#pragma once
#include <vector>
#include <functional>

namespace rev {
	class UniformEnt;
	class GLProgram;
	using UniformSetF = std::function<void (const void*, UniformEnt&)>;
	using UniformSetF_V = std::vector<UniformSetF>;
	struct ISystemUniform {
		virtual ~ISystemUniform() {}
		virtual void applyUniform(UniformEnt&, const GLProgram&) const {}
		virtual void extractUniform(UniformSetF_V&, const GLProgram&) const {}
	};
}
