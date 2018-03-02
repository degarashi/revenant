#pragma once
#include "spine/rflag.hpp"
#include "beat/pose2d.hpp"
#include "frea/matrix.hpp"
#include "handle.hpp"
#include "spine/resmgr.hpp"
#include "spine/singleton.hpp"

namespace rev {
	namespace debug {
		namespace inner {
			bool _Edit(Camera2D&);
		}
	}
	//! 2Dカメラ姿勢クラス
	/*!
		2Dゲームの他にミニマップ表示などで使用
		姿勢の保持はPoseクラスが行い，カメラ固有の変数だけを持つ
	*/
	class Camera2D : public lubee::CheckAlign<Camera2D>, public Resource {
		private:
			struct Pose;
			struct Getter : spi::RFlag_Getter<uint32_t> {
				using RFlag_Getter::operator ();
				counter_t operator()(const beat::g2::Pose& pose, Pose*, const Camera2D&) const {
					return *pose.getAccum();
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
			friend bool debug::inner::_Edit(Camera2D&);
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

			bool operator == (const Camera2D& c) const noexcept;
			bool operator != (const Camera2D& c) const noexcept;
			DEF_RESOURCE_EQUAL
	};
	#define mgr_cam2d (::rev::Camera2DMgr::ref())
	class Camera2DMgr:
		public spi::ResMgr<Camera2D, lubee::AlignedPool<Camera2D>>,
		public spi::Singleton<Camera2DMgr>
	{};
}
#include "luaimport.hpp"
DEF_LUAIMPORT(rev::Camera2D)
