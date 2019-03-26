#pragma once
#include "ft_dep.hpp"
#include "frea/src/vector.hpp"
#include "spine/src/resmgr_named.hpp"
#include "singleton_data_lazy.hpp"

namespace rev {
	//! CharCodeとフォントテクスチャ対応付け (全Face共通)
	using FontChMap = std::unordered_map<CharID, CharPos>;
	// (FaceNameを複数箇所で共有する都合上)
	using String_SP = std::shared_ptr<FontName>;
	struct Face {
		struct DepPair {
			FontArray_Dep		dep;
			CharPlane			cplane;

			DepPair(const String_SP& name, const lubee::PowSize& sfcSize, CCoreID cid);
			DepPair(DepPair&&) = default;
		};
		using DepMap = std::unordered_map<CCoreID, DepPair>;
		DepMap					depMap;
		String_SP				faceName;
		CCoreID					coreID;		// family=0としたCoreID
		const lubee::PowSize&	sfcSize;
		FontChMap&				fontMap;

		Face(Face&& f) = default;
		Face(const String_SP& name, const lubee::PowSize& size, CCoreID cid, FontChMap& m);
		bool operator == (const std::string& name) const;
		bool operator != (const std::string& name) const;
		bool operator == (CCoreID cid) const;
		bool operator != (CCoreID cid) const;
		const CharPos* getCharPos(CharID cid);
		DepPair& getDepPair(CCoreID coreID);
	};

	namespace drawtag {
		//! フォント描画用セッティング Tag
		struct text {};
	}
	struct DrawTag;
}

namespace rev {
	//! 文章の描画に必要なフォントや頂点を用意
	/*!
		TriangleList形式。とりあえず改行だけ対応
		折り返し表示機能は無し
		全体の位置やサイズはシェーダーのconst変数で指定
		オフセットの基準をXYそれぞれで始点、終点、中央　のいずれか選択
		---------------- Uniform変数 ----------------
		Texture: EUnif::Diffuse
	*/
	class TextObj {
		using CPosL = std::vector<const CharPos*>;
		struct DrawSet {
			HPrim	primitive;
			HTex	hTex;
			int		nChar;	//!< スペースなど制御文字を除いた文字数
		};
		using DrawSetL = std::vector<DrawSet>;

		/*! GLリソース再構築時の為に元の文字列も保存
			このクラス自体はテクスチャを持たないのでIGLResourceは継承しない */
		std::u32string	_text;
		DrawSetL		_drawSet;
		CCoreID			_coreID;
		String_SP		_faceName;
		lubee::SizeF	_rectSize;

		//! フォントテクスチャと描画用頂点を用意
		void _init(Face &face);
		// デフォルト描画シェーダー
		using DefaultTech = SingletonDataLazy<ITech, TextObj>;
		const static DefaultTech	s_defaultTech;
		using DefaultFilter = SingletonDataLazy<TextureFilter, TextObj>;
		const static DefaultFilter	s_filter;

		public:
			TextObj(TextObj&& t) = default;
			/*! \param[in] dep フォントデータを生成するための環境依存クラス
				\param[in] s 生成する文字列 */
			TextObj(Face& face, CCoreID coreID, std::u32string&& s);
			// FontGenから呼び出す
			void onCacheLost();
			void onCacheReset(Face& face);
			// ------- clearCache(bRestore=true)時 -------
			// FontGenから呼び出してFaceIDを再設定する用
			CCoreID& refCoreID();
			const String_SP& getFaceName() const;
			/*!
				上位クラスで位置調整など行列をセットしてからメソッドを呼ぶ
				\param[in]	customDraw false=デフォルトのシェーダーでテキスト描画
			*/
			void draw(IEffect& gle) const;
			const lubee::SizeF& getSize() const;
			void exportDrawTag(DrawTag& d) const;
			static HTech MakeData(DefaultTech*);
			static HTexF MakeData(DefaultFilter*);
			static HTech GetDefaultTech();
	};

	#define mgr_text (::rev::FontGen::ref())
	//! フォント作成クラス: 共通
	/*! Depで生成するのはあくまでもフォントデータのみであって蓄積はこのクラスで行う */
	class FontGen : public spi::ResMgrName<TextObj, std::u32string>, public spi::Singleton<FontGen> {
		private:
			//! フォントの名前リスト
			/*! そんなに数いかないと思うのでvectorを使う */
			using FaceList = std::vector<Face>;
			//! フォント名リスト (通し番号)
			FaceList	_faceL;
			//! [CCoreID + CharCode]と[GLTexture + UV + etc..]を関連付け
			FontChMap	_fontMap;
			// OpenGLサーフェスのサイズは2の累乗サイズにする (余った領域は使わない)
			lubee::PowSize	_sfcSize;

			/*! 既にFaceはmakeCoreIDで作成されてる筈 */
			Face& _getArray(CCoreID cid);
			//! 文字列先頭にCCoreIDの文字列を付加したものを返す
			/*! ハッシュキーはUTF32文字列に統一 */
			static std::u32string _MakeTextTag(CCoreID cid, const std::u32string& s);

			template <class S>
			CCoreID _makeCoreID(const S& name, CCoreID cid);

		public:
			/*! sfcSizeは強制的に2の累乗サイズに合わせられる
				\param[in] sfcSize	フォントを蓄えるOpenGLサーフェスのサイズ */
			FontGen(const lubee::PowSize& sfcSize);
			//! フォントの設定から一意のIDを作る
			/*! \param[in] cid フォントの見た目情報。FaceIDは無視される
				\param[in] name フォントファミリの名前
				\return FaceIDを更新したcid */
			CCoreID makeCoreID(const std::string& name, CCoreID cid);
			// 上記のnameがString_SPバージョン
			/*! 動作は同じだが、もしFace名が無ければそのポインタを受け継いでFaceListに加える */
			CCoreID makeCoreID(const String_SP& name, CCoreID cid);
			//! キャッシュを全て破棄
			/*! 文字列ハンドルは有効
				\param[in] bRestore trueならキャッシュを再確保する */
			void clearCache(bool bRestore);

			// 同じFaceの同じ文字列には同一のハンドルが返されるようにする
			/*! \param[in] cid makeCoreIDで作成したFaceID設定済みID
				\param[in] s 表示したい文字列 */
			HText createText(CCoreID cid, To32Str str);

			// デバイスロストで処理が必要なのはテクスチャハンドルだけなので、
			// onDeviceLostやonDeviceResetは特に必要ない
	};
}

