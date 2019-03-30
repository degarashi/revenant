#pragma once
#include "freetype/dep.hpp"
#include "frea/src/vector.hpp"
#include "spine/src/resmgr_named.hpp"
#include "singleton_data_lazy.hpp"

namespace rev {
	struct DrawTag;
	namespace detail {
		//! CharCodeとフォントテクスチャ対応付け (全Face共通)
		using FontChMap = std::unordered_map<CharId, CharPos>;
		// (FaceNameを複数箇所で共有する都合上)
		using FontName_S = std::shared_ptr<FontName>;
		struct Face {
			struct DepPair {
				FontArray_Dep		dep;
				CharPlane			cplane;

				DepPair(const FontName_S &name, const lubee::PowSize &sfcSize, FontId fid);
				DepPair(DepPair&&) = default;
			};
			using DepMap = std::unordered_map<FontId, DepPair>;
			DepMap					depMap;
			FontName_S				faceName;
			FontId					fontId;		// family=0としたFontId
			const lubee::PowSize&	sfcSize;
			FontChMap				&fontMap;

			Face(Face&& f) = default;
			Face(const FontName_S &name, const lubee::PowSize &size, FontId fid, FontChMap &m);
			bool operator == (const std::string &name) const;
			bool operator != (const std::string &name) const;
			bool operator == (FontId fid) const;
			bool operator != (FontId fid) const;
			const CharPos* getCharPos(CharId cid);
			DepPair& getDepPair(FontId fontId);
		};
		struct TextObjPrivate {};
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
			private:
				//! テクスチャ一枚で描画できる文字の集合
				struct DrawSet {
					HPrim	primitive;
					HTex	hTex;
					//! スペースなど制御文字を除いた文字数
					size_t	nChar;
				};
				using DrawSetV = std::vector<DrawSet>;

				/*! GLリソース再構築時の為に元の文字列も保存
					このクラス自体はテクスチャを持たないのでIGLResourceは継承しない */
				std::u32string	_text;
				FontId			_fontId;
				FontName_S		_faceName;
				struct {
					DrawSetV		drawSet;
					lubee::SizeF	rectSize;
				} _cache;

				void _prepareTextureAndVertex(Face &face);
				// デフォルト描画シェーダー
				using DefaultTech = SingletonDataLazy<ITech, TextObj>;
				const static DefaultTech	s_defaultTech;
				using DefaultFilter = SingletonDataLazy<TextureFilter, TextObj>;
				const static DefaultFilter	s_filter;

				friend DefaultTech;
				friend DefaultFilter;
				// ---- fot SingletonDataLazy ----
				static HTech MakeData(DefaultTech*);
				static HTexF MakeData(DefaultFilter*);

			public:
				TextObj(TextObj &&t) = default;
				/*! \param[in] dep フォントデータを生成するための環境依存クラス
					\param[in] s 生成する文字列 */
				TextObj(Face &face, FontId fontId, std::u32string &&s);
				// TextGenから呼び出す
				void onCacheLost(TextObjPrivate);
				void onCacheReset(TextObjPrivate, Face &face);
				// ------- clearCache(bRestore=true)時 -------
				// TextGenから呼び出してFaceIdを再設定する用
				FontId& refFontId(TextObjPrivate);
				const FontName_S& getFaceName() const;
				/*!
					上位クラスで位置調整など行列をセットしてからメソッドを呼ぶ
					\param[in]	customDraw false=デフォルトのシェーダーでテキスト描画
				*/
				void draw(IEffect &gle) const;
				const lubee::SizeF& getDrawSize() const;
				void exportDrawTag(DrawTag &d) const;

				static HTech GetDefaultTech();
		};
	}

	#define mgr_text (::rev::TextGen::ref())
	//! フォント作成クラス: 共通
	/*! Depで生成するのはあくまでもフォントデータのみであって蓄積はこのクラスで行う */
	class TextGen :
		public spi::ResMgrName<detail::TextObj, std::u32string>,
		public spi::Singleton<TextGen>
	{
		private:
			//! フォントの名前リスト
			/*! そんなに数いかないと思うのでvectorを使う */
			using FaceList = std::vector<detail::Face>;
			//! フォント名リスト (通し番号)
			FaceList			_faceL;
			//! [FontId + CharCode]と[GLTexture + UV + etc..]を関連付け
			detail::FontChMap	_fontMap;
			// OpenGLサーフェスのサイズは2の累乗サイズにする (余った領域は使わない)
			lubee::PowSize		_sfcSize;

			/*!
				既にFaceはmakeFontIdで作成されてる筈なので、
				Idに対応するFaceが存在しなければエラー
			*/
			detail::Face& _getFace(FontId fid);
			//! 文字列先頭にFontIdの文字列を付加したものを返す
			/*! ハッシュキーはUTF32文字列に統一 */
			static std::u32string _MakeTextTag(FontId fid, const std::u32string& s);

			template <class S>
			FontId _makeFontId(const S& name, FontId fid);

		public:
			/*! sfcSizeは強制的に2の累乗サイズに合わせられる
				\param[in] sfcSize	フォントを蓄えるOpenGLサーフェスのサイズ */
			TextGen(lubee::PowSize sfcSize);
			//! フォントの設定から一意のIDを作る
			/*! \param[in] fid フォントの見た目情報。FaceIdは無視される
				\param[in] name フォントファミリの名前
				\return FaceIdを更新したfid */
			FontId makeFontId(const FontName &name, FontId fid);
			// 上記のnameがdetail::FontName_Sバージョン
			/*! 動作は同じだが、もしFace名が無ければそのポインタを受け継いでFaceListに加える */
			FontId makeFontId(const detail::FontName_S &name, FontId fid);
			//! キャッシュを全て破棄
			/*! 文字列ハンドルは有効
				\param[in] bRestore trueならキャッシュを再確保する */
			void clearCache(bool bRestore);

			// 同じFaceの同じ文字列には同一のハンドルが返されるようにする
			/*! \param[in] fid makeFontIdで作成したFaceId設定済みID
				\param[in] s 表示したい文字列 */
			HText createText(FontId fid, To32Str str);

			// デバイスロストで処理が必要なのはテクスチャハンドルだけなので、
			// onDeviceLostやonDeviceResetは特に必要ない
	};
}

