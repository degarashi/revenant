#pragma once
#include "spine/rflag.hpp"
#include "lubee/alignedalloc.hpp"
#include "frea/matrix.hpp"
#include "frea/angle.hpp"
#include "frea/quaternion.hpp"

namespace rev {
	//! 3次元姿勢クラス
	class Pose3D : public lubee::CheckAlign<Pose3D> {
		private:
			constexpr static float TURN_THRESHOLD = 1e-5f;
			using AQuat = frea::AQuat;
			using Vec3 = frea::Vec3;
			using AVec3 = frea::AVec3;
			using AMat4 = frea::AMat4;
			using RadF = frea::RadF;
			#define SEQ \
				((Offset)(AVec3)) \
				((Rotation)(AQuat)) \
				((Scaling)(AVec3)) \
				((Accum)(uint32_t)(Offset)(Rotation)(Scaling)) \
				((ToWorld)(AMat4)(Offset)(Rotation)(Scaling)) \
				((ToLocal)(AMat4)(ToWorld))
			RFLAG_DEFINE(Pose3D, SEQ)

			template <class Ar>
			friend void serialize(Ar&, Pose3D&);

		public:
			RFLAG_SETMETHOD_DEFINE(SEQ)
			RFLAG_GETMETHOD_DEFINE(SEQ)
			RFLAG_REFMETHOD_DEFINE(SEQ)
			RFLAG_SETMETHOD(Accum)
			#undef SEQ

			Pose3D() = default;
			Pose3D(const Pose3D& p) = default;
			Pose3D(const AVec3& pos, const AQuat& rot, const AVec3& sc);
			Pose3D(const AMat4& m);
			void identity();

			AVec3 getUp() const;
			AVec3 getRight() const;
			AVec3 getDir() const;
			void setAll(const AVec3& ofs, const AQuat& q, const AVec3& sc);

			// Rotationに変更を加える
			void addAxisRotation(const AVec3& axis, RadF radf);
			// Positionに変更を加える
			void addOffset(const AVec3& ad);

			// ---- helper function ----
			//! 前方への移動(XZ平面限定)
			void moveFwd2D(float speed);
			//! サイド移動(XZ平面限定)
			void moveSide2D(float speed);
			//! 前方への移動(軸フリー)
			void moveFwd3D(float speed);
			//! サイド移動(軸フリー)
			void moveSide3D(float speed);

			//! 方向転換(軸指定)
			void turnAxis(const AVec3& axis, RadF rad);
			//! Yaw Pitch Roll指定の回転
			void turnYPR(RadF yaw, RadF pitch, RadF roll);
			//! 差分入力
			void addRotation(const AQuat& q);
			//! 補間付き回転
			/*! 3軸の目標距離を合計した物が閾値以下ならtrueを返す */
			bool lerpTurn(const AQuat& q_tgt, float t, float threshold=TURN_THRESHOLD);
			//! Upベクトルをrollが0になるよう補正
			void adjustNoRoll();

			Pose3D& operator = (const Pose3D& ps) = default;
			Pose3D& operator = (const AMat4& m);
			// ---- compare method ----
			bool operator == (const Pose3D& ps) const noexcept;
			bool operator != (const Pose3D& ps) const noexcept;

			Pose3D lerp(const Pose3D& p1, float t) const;
			friend std::ostream& operator << (std::ostream&, const Pose3D&);

			// ---- Lua互換用メソッド ----
			bool equal(const Pose3D& p) const noexcept;
			std::string toString() const;
	};
	std::ostream& operator << (std::ostream& os, const Pose3D& ps);
}
