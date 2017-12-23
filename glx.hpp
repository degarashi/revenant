#pragma once
#include "gl_if.hpp"
#include "glx_if.hpp"
#include "glx_parse.hpp"
#include "spine/object_pool.hpp"
#include "gl_buffer.hpp"
#include "resmgr_app.hpp"
#include "drawtoken/viewport.hpp"
#include "glx_const.hpp"
#include "gl_state.hpp"
#include "glx_block.hpp"

namespace rev {
	#define unif_pool (::rev::UnifPool::ref())
	struct UnifPool :
		spi::ObjectPool<draw::TokenBuffer>,
		spi::Singleton<UnifPool>
	{
		using spi::ObjectPool<draw::TokenBuffer>::ObjectPool;
	};

	void OutputCommentBlock(std::ostream& os, const std::string& msg);
	using GLState_SP = std::shared_ptr<GLState>;
	using GLState_SPV = std::vector<GLState_SP>;
	GLState_SP MakeValueSetting(const parse::ValueSetting& s);
	GLState_SP MakeBoolSetting(const parse::BoolSetting& s);

	class UniformMap {
		private:
			//! [UniformId -> Token]
			using Map = std::unordered_map<GLint, draw::TokenBuffer*>;
			Map		_map;
		public:
			// umに既にidが登録されてないかチェックし、されていればそれを、無ければpoolからメモリを確保し返す
			draw::TokenBuffer* makeTokenBuffer(GLint id);
			void copyFrom(const UniformMap& other);
			void moveTo(draw::TokenML& ml);
			void clear();
			bool empty() const noexcept;
	};

	//! Tech | Pass の分だけ作成
	class TPStructR {
		public:
			// OpenGLのレンダリング設定
			using UniIdSet = std::unordered_set<GLint>;
			using MacroMap = std::unordered_map<std::string, std::string>;
			using AttrL = std::vector<const parse::AttrEntry*>;
			using VaryL = std::vector<const parse::VaryEntry*>;
			using ConstL = std::vector<const parse::ConstEntry*>;
			using UnifL = std::vector<const parse::UnifEntry*>;

		private:
			HProg			_prog;
			parse::BlockSet_SP	_block;
			// --- 関連情報(ゼロから構築する場合の設定項目) ---
			//! Attribute: 頂点セマンティクスに対する頂点Id
			VSemAttrV		_vattr;

			//! Setting: Uniformデフォルト値(texture, vector, float, bool)設定を含む。GLDeviceの設定クラスリスト
			GLState_SPV		_setting;

			UniIdSet		_noDefValue;	//!< Uniform非デフォルト値エントリIdセット (主にユーザーの入力チェック用)
			UniformMap		_defaultValue;	//!< Uniformデフォルト値と対応するId
			bool			_bInit = false;	//!< lost/resetのチェック用 (Debug)

			// ----------- GLXStructから読んだデータ群 -----------
			AttrL			_attrL;
			VaryL			_varyL;
			ConstL			_constL;
			UnifL			_unifL;

		public:
			TPStructR() = default;
			//! エフェクトファイルのパース結果を読み取る
			TPStructR(const parse::BlockSet_SP& bs, const parse::TPStruct& tech, const parse::TPStruct& pass);

			//! OpenGL関連のリソースを解放
			/*! GLResourceの物とは別。GLEffectから呼ぶ */
			void ts_onDeviceLost();
			void ts_onDeviceReset(const IEffect& e);

			bool hasSetting(const GLState& s) const;

			const UniformMap& getUniformDefault() const noexcept;
			const UniIdSet& getRequiredUniformEntries() const noexcept;
			const VSemAttrV& getVAttr() const noexcept;

			const HProg& getProgram() const noexcept;
			//! OpenGLに設定を適用
			void applySetting() const;
			//! 設定差分を求める
			static GLState_SPV CalcDiff(const TPStructR& from, const TPStructR& to);
	};
	//! GLXエフェクト管理クラス
	class GLEffect : public IEffect, public std::enable_shared_from_this<GLEffect> {
		public:
			//! [UniformId -> TextureActiveIndex]
			using TexIndex = std::unordered_map<GLint, GLint>;
			//! [(TechId|PassId) -> ProgramClass]
			using TechMap = std::unordered_map<GL16Id, TPStructR>;
			using TexMap = std::unordered_map<GL16Id, TexIndex>;
			//! Tech名とPass名のセット
			using TechName = std::vector<std::vector<std::string>>;
			using TPRef = spi::Optional<const TPStructR&>;

		private:
			parse::BlockSet_SP	_blockSet;
			TechMap			_techMap;			//!< ゼロから設定を構築する場合の情報や頂点セマンティクス
			TechName		_techName;
			TexMap			_texMap;
			bool			_bInit = false;		//!< deviceLost/Resetの状態区別
			diff::Effect	_diffCount;			/*!< バッファのカウントクリアはclearTask()かbeginTask()の呼び出しタイミング */

			struct Current {
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
				} vertex, vertex_prev;
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
				} index, index_prev;
				using HFb_OP = spi::Optional<HFb>;
				HFb_OP				hFb;			//!< 描画対象のフレームバッファ (無効ならデフォルトターゲット)
				HFb					hFbPrev;		//!< 今現在OpenGLで有効になっているフレームバッファ
				using VP_OP = spi::Optional<draw::Viewport>;
				VP_OP				viewport;
				//! 前回とのバッファの差異
				/*! Vertex, Indexバッファ情報を一時的にバックアップして差異の検出に備える */
				diff::Buffer getDifference();

				// Tech, Pass何れかを変更したらDraw変数をクリア
				// passをセットしたタイミングでProgramを検索し、tpsにセット
				GLint_OP				tech,
									pass;

				TexIndex*			pTexIndex;
				bool				bDefaultParam;	//!< Tech切替時、trueならデフォルト値読み込み
				TPRef				tps;			//!< 現在使用中のTech
				UniformMap			uniMap;			//!< 現在設定中のUniform
				draw::TokenML		tokenML;

				void reset();
				//! Tech/Passの切り替えで無効になる変数をリセット
				void _clean_drawvalue();
				void setTech(GLint idTech, bool bDefault);
				void setPass(GLint idPass, TechMap& tmap, TexMap& texMap);
				void _outputDrawCall(draw::VStream& vs);
				void outputFramebuffer();
				//! DrawCallに関連するAPI呼び出しTokenを出力
				/*! Vertex,Index BufferやUniform変数など */
				void outputDrawCall(GLenum mode, GLint first, GLsizei count);
				void outputDrawCallIndexed(GLenum mode, GLsizei count, GLenum sizeF, GLuint offset);
			} _current;

			using UnifIdV = std::vector<GLint>;
			using UnifIdM = std::unordered_map<GL16Id, UnifIdV>;
			using IdPair = std::pair<int,int>;
			using TechIdV = std::vector<IdPair>;

			struct {
				const StrV*			src = nullptr;
				UnifIdM				result;		// [Tech|Pass]->[size(src)]
				const UnifIdV*		resultCur;	// current tech-pass entry
			} _unifId;

			struct {
				const StrPairV*		src = nullptr;
				TechIdV				result;
			} _techId;

			GLint_OP _getPassId(int techId, const std::string& pass) const;
			IdPair _getTechPassId(IdValue id) const;

			/*! 引数はコンパイラで静的に確保される定数を想定しているのでポインタで受け取る
				動的にリストを削除したりはサポートしない */
			void _setConstantUniformList(const StrV* src);
			void _setConstantTechPassList(const StrPairV* src);
			void _clearFramebuffer(draw::TokenML& ml);
		protected:
			virtual void _prepareUniforms();
		public:
			//! Effectファイル(gfx)を読み込む
			/*! フォーマットの解析まではするがGLリソースの確保はしない */
			GLEffect(const std::string& name);
			void onDeviceLost() override;
			void onDeviceReset() override;
			//! GLEffectで発生する例外基底
			struct EC_Base : std::runtime_error {
				using std::runtime_error::runtime_error;
			};
			// ----------------- Exceptions -----------------
			//! 該当するTechが無い
			struct EC_TechNotFound : EC_Base { using EC_Base::EC_Base; };
			//! 範囲外のPass番号を指定
			struct EC_PassOutOfRange : EC_Base { using EC_Base::EC_Base; };
			//! 頂点Attributeにデータがセットされてない
			struct EC_EmptyAttribute : EC_Base { using EC_Base::EC_Base; };
			//! Uniformにデータがセットされてない
			struct EC_EmptyUniform : EC_Base { using EC_Base::EC_Base; };
			//! Macroにデータがセットされてない
			struct EC_EmptyMacro : EC_Base { using EC_Base::EC_Base; };
			//! GLXファイルの文法エラー
			struct EC_GLXGrammar : EC_Base { using EC_Base::EC_Base; };
			//! 該当するGLXファイルが見つからない
			struct EC_FileNotFound : EC_Base {
				EC_FileNotFound(const std::string& fPath);
			};

			// ----------------- Tech&Pass -----------------
			//! Technique
			GLint_OP getTechId(const std::string& tech) const override;
			GLint_OP getPassId(const std::string& pass) const override;
			GLint_OP getPassId(const std::string& tech, const std::string& pass) const override;
			GLint_OP getCurTechId() const override;
			GLint_OP getCurPassId() const override;
			void setTechPassId(IdValue id) override;
			//! TechId, PassIdに該当するProgramハンドルを返す
			/*! \param[in] techId (-1 = currentTechId)
				\param[in] passId (-1 = currentPassId)
				\return 該当があればそのハンドル、なければ無効なハンドル */
			HProg getProgram(int techId=-1, int passId=-1) const override;
			/*!
				\param[in]		id		TechniqueId
				\param[in]		bReset	Tech切替時に初期値をセットするか
			*/
			void setTechnique(GLint id, bool bReset) override;
			//! Pass指定
			void setPass(int id) override;

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

			// ----------------- Uniform Value -----------------
			//! Uniform変数設定 (Tech/Passで指定された名前とセマンティクスのすり合わせを行う)
			GLint_OP getUniformId(const std::string& name) const override;
			GLint_OP getUnifId(IdValue id) const override;
			using IEffect::_makeUniformToken;
			draw::TokenBuffer& _makeUniformTokenBuffer(GLint id) override;
			void _makeUniformToken(draw::TokenDst& dst, GLint id, const bool* b, int n, bool) const override;
			void _makeUniformToken(draw::TokenDst& dst, GLint id, const float* fv, int n, bool) const override;
			void _makeUniformToken(draw::TokenDst& dst, GLint id, const double* fv, int n, bool) const override;
			void _makeUniformToken(draw::TokenDst& dst, GLint id, const int* iv, int n, bool) const override;
			void _makeUniformToken(draw::TokenDst& dst, GLint id, const HTex* hTex, int n, bool) const override;

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
