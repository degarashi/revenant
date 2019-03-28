#pragma once
#include "spine/src/rflag.hpp"
#include "beat/src/pose2d.hpp"
#include "frea/src/matrix.hpp"
#include "handle/camera.hpp"
#include "spine/src/resmgr.hpp"
#include "spine/src/singleton.hpp"
#include "debuggui_if.hpp"
#include "resource.hpp"

namespace rev {
	//! 2Dカメラ姿勢クラス
	/*!
		2Dゲームの他にミニマップ表示などで使用
		姿勢の保持はPoseクラスが行い，カメラ固有の変数だけを持つ
	*/
	class Camera2D :
		public lubee::CheckAlign<Camera2D>,
		public lubee::AAllocator<Camera2D>,
		public Resource,
		public IDebugGui
	{
		private:
			struct Pose;
			struct Getter : spi::RFlag_Getter<uint32_t> {
				using RFlag_Getter::operator ();
				counter_t operator()(const beat::g2::Pose& pose, Pose*, const Camera2D&) const {
					if(const auto ac = pose.getAccum())
						return *ac;
					return ~0;
				}
			};
			using AMat3 = frea::AMat3;
			using Vec2 = frea::Vec2;
			using View_t = spi::AcCheck<AMat3, Getter>;
			using Accum_t = spi::AcCheck<lubee::Wrapper<uint32_t>, Getter>;
			#define SEQ \
				((Pose)(beat::g2::Pose)) \
				((View)(View_t)(Pose)) \
				((ViewInv)(AMat3)(View)) \
				((AspectRatio)(float)) \
				((Proj)(AMat3)(AspectRatio)) \
				((ViewProj)(AMat3)(View)(Proj)) \
				((ViewProjInv)(AMat3)(ViewProj)) \
				((Accum)(Accum_t)(Pose)(AspectRatio))
			RFLAG_DEFINE(Camera2D, SEQ)
			RFLAG_SETMETHOD(Accum)
		public:
			RFLAG_GETMETHOD_DEFINE(SEQ)
			RFLAG_SETMETHOD_DEFINE(SEQ)
			RFLAG_REFMETHOD(Pose)
			#undef SEQ

			Camera2D();
			//! Projection後の座標をワールド座標へ変換
			Vec2 vp2w(const Vec2& pos) const;
			//! View座標をワールド座標へ変換
			Vec2 v2w(const Vec2& pos) const;
			const char* getResourceName() const noexcept override;
			DEF_DEBUGGUI_NAME
			DEF_DEBUGGUI_PROP

			bool operator == (const Camera2D& c) const noexcept;
			bool operator != (const Camera2D& c) const noexcept;
			DEF_RESOURCE_EQUAL
	};
}
#include "lua/import.hpp"
DEF_LUAIMPORT(rev::Camera2D)
