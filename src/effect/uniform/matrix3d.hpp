#pragma once
#include "../uniform_setter.hpp"
#include "../../handle/camera.hpp"
#include "spine/src/rflag.hpp"
#include "frea/src/matrix.hpp"
#include "lubee/src/alignedalloc.hpp"

namespace rev {
	class U_Matrix3D :
		public lubee::CheckAlign<U_Matrix3D>,
		public IUniformSetter
	{
		private:
			using V3 = frea::Vec3;
			using M4 = frea::AMat4;
			using M4V = std::vector<M4, lubee::AlignedPool<M4>>;
			struct Camera;
			struct Getter : spi::RFlag_Getter<uint32_t> {
				using RFlag_Getter::operator();
				counter_t operator()(const HCam3& c, Camera*, const U_Matrix3D&) const;
			};
			using Transform_t = spi::AcCheck<M4, Getter>;
			#define SEQ_SYSUNI3D \
				((World)(M4)) \
				((WorldInv)(M4)(World)) \
				((Camera)(HCam3)) \
				((ViewInv)(Transform_t)(Camera)) \
				((ProjInv)(Transform_t)(Camera)) \
				((Transform)(Transform_t)(World)(Camera)) \
				((TransformInv)(M4)(Transform)) \
				((Joint)(M4V))
			RFLAG_DEFINE(U_Matrix3D, SEQ_SYSUNI3D)
			RFLAG_SETMETHOD(Transform)
		public:
			RFLAG_GETMETHOD_DEFINE(SEQ_SYSUNI3D)
			RFLAG_REFMETHOD_DEFINE(SEQ_SYSUNI3D)
			RFLAG_SETMETHOD_DEFINE(SEQ_SYSUNI3D)
			#undef SEQ_SYSUNI3D

			U_Matrix3D();
			void resetWorld();
			UniformSetF getUniformF(const GLProgram& prog) const override;
	};
}
