#pragma once
#include "spine/rflag.hpp"
#include "handle/camera.hpp"
#include "frea/matrix.hpp"
#include "lubee/alignedalloc.hpp"
#include "uniform_setter.hpp"

namespace rev {
	class U_Matrix3D :
		public lubee::CheckAlign<U_Matrix3D>,
		public IUniformSetter
	{
		private:
			struct Camera;
			struct Getter : spi::RFlag_Getter<uint32_t> {
				using RFlag_Getter::operator();
				counter_t operator()(const HCam3& c, Camera*, const U_Matrix3D&) const;
			};
			using Transform_t = spi::AcCheck<frea::AMat4, Getter>;
			#define SEQ_SYSUNI3D \
				((World)(frea::AMat4)) \
				((WorldInv)(frea::AMat4)(World)) \
				((Camera)(HCam3)) \
				((ViewInv)(Transform_t)(Camera)) \
				((ProjInv)(Transform_t)(Camera)) \
				((Transform)(Transform_t)(World)(Camera)) \
				((TransformInv)(frea::AMat4)(Transform))
			RFLAG_DEFINE(U_Matrix3D, SEQ_SYSUNI3D)
			RFLAG_SETMETHOD(Transform)
		public:
			RFLAG_GETMETHOD_DEFINE(SEQ_SYSUNI3D)
			RFLAG_REFMETHOD_DEFINE(SEQ_SYSUNI3D)
			RFLAG_SETMETHOD_DEFINE(SEQ_SYSUNI3D)
			#undef SEQ_SYSUNI3D

			U_Matrix3D();
			void extractUniform(UniformSetF_V& dst, const GLProgram& prog) const override;
	};
}
