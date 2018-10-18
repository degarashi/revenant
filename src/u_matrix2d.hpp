#pragma once
#include "spine/src/rflag.hpp"
#include "handle/camera.hpp"
#include "frea/src/matrix.hpp"
#include "uniform_setter.hpp"

namespace rev {
	//! システムuniform変数をセットする(2D)
	/*!
		変数リスト:
		mat3 sys_mTrans2D;		// scale * rotation * offset
	*/
	class U_Matrix2D : public IUniformSetter {
		private:
			struct Camera;
			struct Getter : spi::RFlag_Getter<uint32_t> {
				using RFlag_Getter::operator();
				counter_t operator()(const HCam2& c, Camera*, const U_Matrix2D&) const;
			};
			using Transform_t = spi::AcCheck<frea::Mat3, Getter>;
			#define SEQ_SYSUNI2D \
				((World)(frea::Mat3)) \
				((WorldInv)(frea::Mat3)(World)) \
				((Camera)(HCam2)) \
				((Transform)(Transform_t)(World)(Camera)) \
				((TransformInv)(frea::Mat3)(Transform))
			RFLAG_DEFINE(U_Matrix2D, SEQ_SYSUNI2D)
			RFLAG_SETMETHOD(Transform)
		public:
			RFLAG_GETMETHOD_DEFINE(SEQ_SYSUNI2D)
			RFLAG_REFMETHOD_DEFINE(SEQ_SYSUNI2D)
			RFLAG_SETMETHOD_DEFINE(SEQ_SYSUNI2D)
			#undef SEQ_SYSUNI2D

			U_Matrix2D();
			UniformSetF getUniformF(const GLProgram& prog) const override;
	};
}
