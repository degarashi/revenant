#pragma once
#include "lubee/alignedalloc.hpp"
#include "lubee/size.hpp"
#include "spine/rflag.hpp"
#include "handle/camera.hpp"
#include "frea/matrix.hpp"

namespace rev {
	class UniformEnt;
	//! (3D/2D共通)
	/*!
		予め変数名がsys_*** の形で決められていて, 存在すれば計算&設定される
		固定変数リスト:
		vec4 sys_screen;		// x=ScreenSizeX, y=ScreenSizeY, z=1DotSizeX, w=1DotSizeY
	*/
	class SystemUniform {
		private:
			lubee::SizeI	_screenSize;
		public:
			const lubee::SizeI& getScreenSize() const;
			void setScreenSize(const lubee::SizeI& s);
			void outputUniforms(UniformEnt& u) const;
			void moveFrom(SystemUniform& prev);
	};
	class SystemUniform3D : public lubee::CheckAlign<SystemUniform3D> {
		private:
			struct Camera;
			struct Getter : spi::RFlag_Getter<uint32_t> {
				using RFlag_Getter::operator();
				counter_t operator()(const HCam3& c, Camera*, const SystemUniform3D&) const;
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
			RFLAG_DEFINE(SystemUniform3D, SEQ_SYSUNI3D)
			RFLAG_SETMETHOD(Transform)
		public:
			RFLAG_GETMETHOD_DEFINE(SEQ_SYSUNI3D)
			RFLAG_REFMETHOD_DEFINE(SEQ_SYSUNI3D)
			RFLAG_SETMETHOD_DEFINE(SEQ_SYSUNI3D)
			#undef SEQ_SYSUNI3D

			SystemUniform3D();
			void outputUniforms(UniformEnt& u) const;
			void moveFrom(SystemUniform3D& prev);
	};
	//! システムuniform変数をセットする(2D)
	/*!
		変数リスト:
		mat3 sys_mTrans2D;		// scale * rotation * offset
	*/
	class SystemUniform2D {
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
			void outputUniforms(UniformEnt& u) const;
			void moveFrom(SystemUniform2D& prev);
	};
}
