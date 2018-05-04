#pragma once
#include "differential.hpp"
#include "gl_types.hpp"
#include "sys_uniform_if.hpp"
#include "handle/opengl.hpp"
#include <unordered_map>

namespace lubee {
	template <class T>
	class Rect;
	using RectI = Rect<int32_t>;
	using RectF = Rect<float>;
}
namespace rev {
	namespace draw {
		struct ClearParam;
	}
	class UniformEnt;
	class FBRect;
	class IEffect :
		public IGLResource,
		public ISystemUniform
	{
		public:
			virtual UniformEnt& refUniformEnt() noexcept = 0;
			virtual HTech setTechnique(const HTech& tech) = 0;
			virtual const HTech& getTechnique() const noexcept = 0;
			virtual void setFramebuffer(const HFb& fb) = 0;
			virtual HFb getFramebuffer() const = 0;
			virtual FBRect setViewport(const FBRect& r) = 0;
			virtual const FBRect& getViewport() const noexcept = 0;
			virtual FBRect setScissor(const FBRect& r) = 0;
			virtual void resetFramebuffer() = 0;
			virtual void setPrimitive(const HPrim& p) noexcept = 0;
			virtual void clearFramebuffer(const draw::ClearParam& param) = 0;
			virtual void draw() = 0;
			virtual void beginTask() = 0;
			virtual void endTask() = 0;
			virtual void execTask() = 0;
			virtual diff::Effect getDifference() const = 0;
	};
}
