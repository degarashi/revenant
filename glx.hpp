#pragma once
#include "glx_if.hpp"
#include "glx_tech.hpp"
#include "resmgr_app.hpp"
#include "drawtoken/viewport.hpp"
#include "drawtoken/tokenml.hpp"

namespace rev {
	namespace draw {
		class VStream;
	}
	struct Primitive;
	using Primitive_SP = std::shared_ptr<Primitive>;
	//! GLXエフェクト管理クラス
	class GLEffect : public IEffect, public std::enable_shared_from_this<GLEffect> {
		public:
			bool			_bInit = false;		//!< deviceLost/Resetの状態区別
			diff::Effect	_diffCount;			/*!< バッファのカウントクリアはclearTask()かbeginTask()の呼び出しタイミング */

			Primitive_SP	_primitive,
							_primitive_prev;
			using HFb_OP = spi::Optional<HFb>;
			HFb_OP				_hFb;			//!< 描画対象のフレームバッファ (無効ならデフォルトターゲット)
			HFb					_hFbPrev;		//!< 今現在OpenGLで有効になっているフレームバッファ
			using VP_OP = spi::Optional<draw::Viewport>;
			VP_OP				_viewport;
			Tech_SP				_tech_sp;		//!< 現在使用中のTech
			UniformEnt			_uniformEnt;
			draw::TokenML		_tokenML;		//!< 描画スレッドに渡す予定のコマンド

			//! 前回とのバッファの差異
			/*! Vertex, Indexバッファ情報を一時的にバックアップして差異の検出に備える */
			diff::Buffer _getDifference();
			void _reset();
			//! Tech/Passの切り替えで無効になる変数をリセット
			void _clean_drawvalue();
			void _outputDrawCall(draw::VStream& vs);
			void _outputFramebuffer();
			//! DrawCallに関連するAPI呼び出しTokenを出力
			/*! Vertex,Index BufferやUniform変数など */
			void _outputDrawCall(GLenum mode, GLint first, GLsizei count);
			void _outputDrawCallIndexed(GLenum mode, GLsizei count, GLenum sizeF, GLuint offset);
			void _clearFramebuffer(draw::TokenML& ml);
		protected:
			virtual void _prepareUniforms();
		public:
			GLEffect();
			void setTechnique(const Tech_SP& tech) override;
			const Tech_SP& getTechnique() const noexcept override;
			void onDeviceLost() override;
			void onDeviceReset() override;
			UniformMap_t& refUniformMap() noexcept override;
			UniformIdMap_t& refUniformIdMap() noexcept override;
			//! GLEffectで発生する例外基底
			struct EC_Base : std::runtime_error {
				using std::runtime_error::runtime_error;
			};
			// ----------------- Exceptions -----------------
			//! 該当するTechが無い
			struct EC_TechNotFound : EC_Base { using EC_Base::EC_Base; };
			//! 頂点Attributeにデータがセットされてない
			struct EC_EmptyAttribute : EC_Base { using EC_Base::EC_Base; };
			//! 必須Uniformにデータがセットされてない
			struct EC_EmptyUniform : EC_Base { using EC_Base::EC_Base; };
			// ----------------- Framebuffer -----------------
			void setFramebuffer(const HFb& fb) override;
			HFb getFramebuffer() const override;
			//! アプリケーション初期化時のデフォルトフレームバッファに戻す
			void resetFramebuffer() override;
			void setViewport(bool bPixel, const lubee::RectF& r) override;

			// ----------------- Primitive -----------------
			void setPrimitive(const Primitive_SP& p) noexcept override;

			// ----------------- Buffer Clear -----------------
			void clearFramebuffer(const draw::ClearParam& param) override;
			// ----------------- Draw call -----------------
			//! IStreamを使用して描画
			/*! \param[in] mode 描画モードフラグ(OpenGL)
				\param[in] count 描画に使用される要素数
				\param[in] offsetElem オフセット要素数 */
			void drawIndexed(GLenum mode, GLsizei count, GLuint offsetElem=0) override;
			//! IStreamを使わず描画
			/*! \param[in] mode 描画モードフラグ(OpenGL)
				\param[in] first 描画を開始する要素オフセット
				\param[in] count 描画に使用される要素数 */
			void draw(GLenum mode, GLint first, GLsizei count) override;

			// ----------------- Task switching -----------------
			// ---- from MainThread ----
			//! バッファを切り替えて古いバッファをクリア
			/*! まだDrawThreadが描画を終えてない場合はブロック */
			void beginTask() override;
			//! OpenGLコマンドをFlushする
			void endTask() override;
			// ---- from DrawThread ----
			void execTask() override;
			//! 1フレームあたりのドローコール回数など
			diff::Effect getDifference() const override;
	};
}
