#pragma once
#include "prog_unif.hpp"
#include "differential.hpp"

namespace lubee {
	template <class T>
	class Rect;
	using RectI = Rect<int32_t>;
	using RectF = Rect<float>;
}
namespace rev {
	struct ITech;
	using Tech_SP = std::shared_ptr<ITech>;
	class VDecl;
	using VDecl_SP = std::shared_ptr<VDecl>;
	namespace draw {
		struct ClearParam;
	}
	struct Primitive;
	using Primitive_SP = std::shared_ptr<Primitive>;
	class SystemUniform2D;
	class SystemUniform3D;
	class IEffect :
		public Prog_Unif,
		public IGLResource
	{
		public:
			virtual void setTechnique(const Tech_SP& tech) = 0;
			virtual void setFramebuffer(const HFb& fb) = 0;
			virtual HFb getFramebuffer() const = 0;
			virtual void setViewport(bool bPixel, const lubee::RectF& r) = 0;
			virtual void resetFramebuffer() = 0;
			virtual void setPrimitive(const Primitive_SP& p) noexcept = 0;
			virtual void clearFramebuffer(const draw::ClearParam& param) = 0;
			virtual void drawIndexed(GLenum mode, GLsizei count, GLuint offsetElem=0) = 0;
			virtual void draw(GLenum mode, GLint first, GLsizei count) = 0;
			virtual void beginTask() = 0;
			virtual void endTask() = 0;
			virtual void execTask() = 0;
			virtual diff::Effect getDifference() const = 0;

			virtual SystemUniform2D& ref2D() {
				AssertF("this class has no SystemUniform2D interface");
			}
			virtual SystemUniform3D& ref3D() {
				AssertF("this class has no SystemUniform3D interface");
			}
			const SystemUniform2D& ref2D() const {
				return const_cast<IEffect*>(this)->ref2D();
			}
			const SystemUniform3D& ref3D() const {
				return const_cast<IEffect*>(this)->ref3D();
			}
			virtual void moveFrom(IEffect& /*prev*/) {}
	};
}
