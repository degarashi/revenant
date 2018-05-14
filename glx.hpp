#pragma once
#include "glx_if.hpp"
#include "glx_tech.hpp"
#include "resmgr_app.hpp"
#include "clear.hpp"
#include "fbrect.hpp"
#include "drawcmd/task.hpp"
#include "uniform_ent.hpp"

namespace rev {
	//! GLXエフェクト管理クラス
	class GLEffect : public IEffect, public std::enable_shared_from_this<GLEffect> {
		private:
			struct DCmd_Clear {
				bool			bColor,
								bDepth,
								bStencil;
				frea::Vec4		color;
				float			depth;
				uint32_t		stencil;
				static void Command(const void* p);
			};
			struct DCmd_Scissor {
				FBRect		rect;
				static void Command(const void* p);
			};
			struct DCmd_Viewport {
				FBRect		rect;
				static void Command(const void* p);
			};

			bool			_bInit = false;		//!< deviceLost/Resetの状態区別

			HPrim			_primitive,
							_primitive_prev;
			using HFb_OP = spi::Optional<HFb>;
			HFb_OP				_hFb;			//!< 描画対象のフレームバッファ (無効ならデフォルトターゲット)
			HFb					_hFbPrev;		//!< 今現在OpenGLで有効になっているフレームバッファ
			FBRect				_viewrect,
								_scissorrect;
			bool				_bView,
								_bScissor;
			HTech				_tech_sp;		//!< 現在使用中のTech
			using UniformEnt_OP = spi::Optional<UniformEnt>;
			UniformEnt_OP		_uniformEnt;
			draw::Task			_task;
			draw::IQueue*		_writeEnt;

			void _reset();
			void _outputFramebuffer();
		public:
			GLEffect();
			void setTechnique(const HTech& tech) override;
			const HTech& getTechnique() const noexcept override;
			void onDeviceLost() override;
			void onDeviceReset() override;
			UniformEnt& refUniformEnt() noexcept override;
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
			FBRect setViewport(const FBRect& r) override;
			const FBRect& getViewport() const noexcept override;
			FBRect setScissor(const FBRect& r) override;

			// ----------------- Primitive -----------------
			void setPrimitive(const HPrim& p) noexcept override;

			// ----------------- Buffer Clear -----------------
			void clearFramebuffer(const ClearParam& param) override;
			// ----------------- Draw call -----------------
			void draw() override;

			// ----------------- Task switching -----------------
			// ---- from MainThread ----
			//! バッファを切り替えて古いバッファをクリア
			/*! まだDrawThreadが描画を終えてない場合はブロック */
			void beginTask() override;
			//! OpenGLコマンドをFlushする
			void endTask() override;
			//! ウィンドウ非アクティブ時、描画タスクをクリア
			void clearTask() override;
			// ---- from DrawThread ----
			void execTask() override;
	};
}
