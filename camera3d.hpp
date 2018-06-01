#pragma once
#include "spine/rflag.hpp"
#include "frea/matrix.hpp"
#include "frea/plane.hpp"
#include "handle/camera.hpp"
#include "beat/pose3d.hpp"
#include "spine/resmgr.hpp"
#include "spine/singleton.hpp"
#include "resource.hpp"
#include "debuggui_if.hpp"

namespace rev {
	/*! 姿勢の保持はPoseクラスが行い，カメラ固有の変数だけを持つ */
	class Camera3D :
		public lubee::CheckAlign<Camera3D>,
		public lubee::AAllocator<Camera3D>,
		public Resource,
		public IDebugGui
	{
		private:
			using AMat4 = frea::AMat4;
			using AVec4 = frea::AVec4;
			using Vec3 = frea::Vec3;
			using Vec2 = frea::Vec2;
			using RadF = frea::RadF;
			using Quat = frea::Quat;
			using Plane = frea::Plane;
			struct Pose;
			struct Getter : spi::RFlag_Getter<uint32_t> {
				using RFlag_Getter::operator ();
				counter_t operator()(const beat::g3::Pose& pose, Pose*, const Camera3D&) const {
					if(const auto ac = pose.getAccum())
						return *ac;
					return ~0;
				}
			};
			using View_t = spi::AcCheck<AMat4, Getter>;
			using Accum_t = spi::AcCheck<lubee::Wrapper<uint32_t>, Getter>;
			#define SEQ \
				((Pose)(beat::g3::Pose)) \
				((View)(View_t)(Pose)) \
				((Fov)(RadF)) \
				((Aspect)(float)) \
				((NearZ)(float)) \
				((FarZ)(float)) \
				((Proj)(AMat4)(Fov)(Aspect)(NearZ)(FarZ)) \
				((ViewProj)(AMat4)(View)(Proj)) \
				((ViewProjInv)(AMat4)(ViewProj)) \
				((Accum)(Accum_t)(Pose)(Fov)(Aspect)(NearZ)(FarZ))
				// ((VFrustum)(boom::geo3d::Frustum)(View)(Fov)(Aspect)(NearZ)(FarZ))
			RFLAG_DEFINE(Camera3D, SEQ)
			RFLAG_SETMETHOD(Accum)
		public:
			using Vec3x2 = std::pair<Vec3, Vec3>;
			Camera3D();

			RFLAG_GETMETHOD_DEFINE(SEQ)
			RFLAG_SETMETHOD_DEFINE(SEQ)
			RFLAG_REFMETHOD_DEFINE(SEQ)
			#undef SEQ

			void setZPlane(float n, float f);

			Plane getNearPlane() const;
			// boom::geo3d::Frustum getNearFrustum() const;
			//! ビューポート座標をワールド座標系に変換
			Vec3 unproject(const Vec3& vsPos) const;		// 単体
			Vec3x2 unprojectVec(const Vec2& vsPos) const;	// (Near,Far)
			//! ビューポート座標からワールド座標(FarPlane位置)を取得
			Vec3 vp2wp(const Vec3& vp) const;
			const char* getResourceName() const noexcept override;

			bool operator == (const Camera3D& c) const noexcept;
			bool operator != (const Camera3D& c) const noexcept;
			DEF_RESOURCE_EQUAL

			DEF_DEBUGGUI_NAME
			DEF_DEBUGGUI_PROP
	};
}
#include "luaimport.hpp"
DEF_LUAIMPORT(rev::Camera3D)
