#pragma once
#include "spine/rflag.hpp"
#include "handle/camera.hpp"
#include "frea/matrix.hpp"
#include "sys_uniform_if.hpp"

namespace rev {
	//! システムuniform変数をセットする(2D)
	/*!
		変数リスト:
		mat3 sys_mTrans2D;		// scale * rotation * offset
	*/
	class SystemUniform2D : public ISystemUniform {
		private:
			struct Camera;
			struct Getter : spi::RFlag_Getter<uint32_t> {
				using RFlag_Getter::operator();
				counter_t operator()(const HCam2& c, Camera*, const SystemUniform2D&) const;
			};
			using Transform_t = spi::AcCheck<frea::Mat3, Getter>;
			#define SEQ_SYSUNI2D \
				((World)(frea::Mat3)) \
				((WorldInv)(frea::Mat3)(World)) \
				((Camera)(HCam2)) \
				((Transform)(Transform_t)(World)(Camera)) \
				((TransformInv)(frea::Mat3)(Transform))
			RFLAG_DEFINE(SystemUniform2D, SEQ_SYSUNI2D)
			RFLAG_SETMETHOD(Transform)
		public:
			RFLAG_GETMETHOD_DEFINE(SEQ_SYSUNI2D)
			RFLAG_REFMETHOD_DEFINE(SEQ_SYSUNI2D)
			RFLAG_SETMETHOD_DEFINE(SEQ_SYSUNI2D)
			#undef SEQ_SYSUNI2D

			SystemUniform2D();
			void outputUniforms(UniformEnt& u) const override;
			void moveFrom(ISystemUniform& prev) override;
	};
}
