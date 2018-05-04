#pragma once

namespace rev {
	class UniformEnt;
	struct ISystemUniform {
		virtual ~ISystemUniform() {}
		virtual void outputUniforms(UniformEnt&) const {}
		virtual void moveFrom(ISystemUniform&) {}
	};
}
