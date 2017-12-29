#pragma once
#include "glx_if.hpp"
#include "glx_tech.hpp"
#include "resmgr_app.hpp"
#include "drawtoken/viewport.hpp"
#include "drawtoken/tokenml.hpp"
#include "glx_const.hpp"
#include "uniform_map.hpp"

namespace rev {
	namespace parse {
		class BlockSet;
		using BlockSet_SP = std::shared_ptr<BlockSet>;
	}
	namespace draw {
		class VStream;
	}

	//! GLXエフェクト管理クラス
	class GLEffect : public IEffect, public std::enable_shared_from_this<GLEffect> {
		public:
			bool			_bInit = false;		//!< deviceLost/Resetの状態区別
			diff::Effect	_diffCount;			/*!< バッファのカウントクリアはclearTask()かbeginTask()の呼び出しタイミング */

			class Vertex {
				private:
					VDecl_SP		_spVDecl;
					HVb				_vbuff[MaxVStream];
				public:
					Vertex();
					void setVDecl(const VDecl_SP& v);
					void setVBuffer(const HVb& hVb, int n);
					void reset();
					void extractData(draw::VStream& dst, const VSemAttrV& vAttr) const;
					bool operator != (const Vertex& v) const;
			} _vertex, _vertex_prev;
			class Index {
				private:
					HIb				_ibuff;
				public:
					Index();
					void setIBuffer(const HIb& hIb);
					const HIb& getIBuffer() const;
					void reset();
					void extractData(draw::VStream& dst) const;
					bool operator != (const Index& idx) const;
			} _index, _index_prev;
			using HFb_OP = spi::Optional<HFb>;
			HFb_OP				_hFb;			//!< 描画対象のフレームバッファ (無効ならデフォルトターゲット)
			HFb					_hFbPrev;		//!< 今現在OpenGLで有効になっているフレームバッファ
			using VP_OP = spi::Optional<draw::Viewport>;
			VP_OP				_viewport;
			Tech_SP				_tech_sp;		//!< 現在使用中のTech
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
			void setTechnique(const Tech_SP& tech) override;
			void onDeviceLost() override;
			void onDeviceReset() override;
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

			// ----------------- Vertex&Index Stream -----------------
			//! 頂点宣言
			/*! \param[in] decl 頂点定義クラスのポインタ(定数を前提) */
			void setVDecl(const VDecl_SP& decl) override;
			void setVStream(const HVb& vb, int n) override;
			void setIStream(const HIb& ib) override;

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
