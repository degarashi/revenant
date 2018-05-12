#pragma once
#include "gl_types.hpp"
#include "gl_format.hpp"
#include "lubee/size.hpp"
#include "lubee/rect.hpp"
#include "abstbuffer.hpp"
#include "uri.hpp"
#include "chunk.hpp"

namespace rev {
	struct TextureBase {
		void use_begin() const;

		//! [mipLevel][Nearest / Linear]
		const static GLuint cs_Filter[3][2];

		GLuint		_idTex;
		int			_iLinearMag,	//!< Linearの場合は1, Nearestは0
					_iLinearMin;
		WrapState	_wrapS,
					_wrapT;
		MipState	_mipLevel;
		GLuint		_actId;		//!< セットしようとしているActiveTextureId (for Use())
		GLuint		_texFlag;	//!< TEXTURE_2D or TEXTURE_CUBE_MAP
		float		_coeff;

		GLuint getTexFlag() const;
		//! テクスチャユニット番号を指定してBind
		void setActiveId(GLuint n);
		GLint getTextureId() const;
		static bool IsMipmap(MipState level);
		bool isMipmap() const;
		bool isCubemap() const;
		void setFilter(bool bLinearMag, bool bLinearMin);
		void setMagMinFilter(bool bLinear);
		void setAnisotropicCoeff(float coeff);
		void setUVWrap(WrapState s, WrapState t);
		void setWrap(WrapState st);
	};
	class PathBlock;
	using ByteBuff = std::vector<uint8_t>;

	namespace draw {
		class IQueue;
	}
	//! OpenGLテクスチャインタフェース
	/*!	フィルターはNEARESTとLINEARしか無いからboolで管理 */
	class IGLTexture :
		public TextureBase,
		public IGLResource,
		public std::enable_shared_from_this<IGLTexture>
	{
		private:
			struct DCmd_Uniform : TextureBase {
				GLint	unifId;
				static void Command(const void* p);
			};
		protected:
			GLuint				_faceFlag;	//!< TEXTURE_2D or TEXTURE_CUBE_MAP_POSITIVE_X
			lubee::SizeI		_size;
			InCompressedFmt_OP	_format;	//!< 値が無効 = 不定

			bool _onDeviceReset();
			IGLTexture(MipState miplevel, InCompressedFmt_OP fmt, const lubee::SizeI& sz, bool bCube);

		public:
			virtual ~IGLTexture();
			IGLTexture(IGLTexture&& t);

			const char* getResourceName() const noexcept override;

			const lubee::SizeI& getSize() const;
			const InCompressedFmt_OP& getFormat() const;
			GLenum getFaceFlag(CubeFace face=CubeFace::PositiveX) const;
			void onDeviceLost() override;

			//! 内容をファイルに保存 (主にデバッグ用)
			void save(const PathBlock& path, CubeFace face=CubeFace::PositiveX);
			DEF_DEBUGGUI_PROP
			DEF_DEBUGGUI_SUMMARY

			bool operator == (const IGLTexture& t) const;
			ByteBuff readData(GLInFmt internalFmt, GLTypeFmt elem, int level=0, CubeFace face=CubeFace::PositiveX) const;
			ByteBuff readRect(GLInFmt internalFmt, GLTypeFmt elem, const lubee::RectI& rect, CubeFace face=CubeFace::PositiveX) const;
			void dcmd_uniform(draw::IQueue& q, const GLint id, int actId) const;
	};
	//! ユーザー定義の空テクスチャ
	/*!
		DeviceLost時の復元は任意
		内部バッファはDeviceLost用であり、DeviceがActiveな時はnone
		フォーマット変換は全てOpenGLにさせる
		書き込み不可の時は最初の一度だけ書き込める
	*/
	class Texture_Mem : public IGLTexture {
		private:
			using Buff_OP = spi::Optional<ByteBuff>;
			using Format_OP = spi::Optional<GLTypeFmt>;
			Buff_OP		_buff;			//!< DeviceLost時用のバッファ
			Format_OP		_typeFormat;	//!< _buffに格納されているデータの形式(Type)

			// bool		_bStream;		//!< 頻繁に書き換えられるか(の、ヒント)
			bool		_bRestore;
			//! テクスチャフォーマットから必要なサイズを計算してバッファを用意する
			const GLFormatDesc& _prepareBuffer();
		public:
			Texture_Mem(bool bCube, GLInSizedFmt fmt, const lubee::SizeI& sz, bool bStream, bool bRestore);
			void onDeviceReset() override;
			void onDeviceLost() override;
			//! テクスチャ全部書き換え = バッファも置き換え
			/*! \param[in] fmt テクスチャのフォーマット
				\param[in] srcFmt 入力フォーマット(Type)
				\param[in] bRestore trueなら内部バッファにコピーを持っておいてDeviceLostに備える
				\param[in] face Cubemapにおける面 */
			void writeData(AB_Byte buff, GLTypeFmt srcFmt, CubeFace face=CubeFace::PositiveX);
			//! 部分的に書き込み
			/*! \param[in] ofsX 書き込み先オフセット X
				\param[in] ofsY 書き込み先オフセット Y
				\param[in] srcFmt 入力フォーマット(Type)
				\param[in] face Cubemapにおける面 */
			void writeRect(AB_Byte buff, const lubee::RectI& rect, GLTypeFmt srcFmt, CubeFace face=CubeFace::PositiveX);
			DEF_DEBUGGUI_NAME
	};
	using Size_Fmt = std::pair<lubee::SizeI, GLInCompressedFmt>;
	Size_Fmt LoadTextureFromBuffer(const IGLTexture& tex, GLenum tflag, GLenum format, const lubee::SizeI& size, const ByteBuff& buff, bool bP2, bool bMip);

	//! URIから2Dテクスチャを読む
	/*!
		DeviceReset時:
		再度URIを参照
	*/
	class Texture_URI : public IGLTexture {
		private:
			HURI				_uri;
		public:
			Texture_URI(const HURI& uri, MipState miplevel, InCompressedFmt_OP fmt);
			void onDeviceReset() override;
			DEF_DEBUGGUI_PROP
			DEF_DEBUGGUI_NAME
	};
	//! 6つの画像ファイルからCubeテクスチャを構成
	class Texture_CubeURI : public IGLTexture {
		private:
			HURI				_uri[6];
		public:
			Texture_CubeURI(
				const HURI& uri0, const HURI& uri1, const HURI& uri2,
				const HURI& uri3, const HURI& uri4, const HURI& uri5,
				MipState miplevel, InCompressedFmt_OP fmt
			);
			void onDeviceReset() override;
			DEF_DEBUGGUI_NAME
	};
}
#include "luaimport.hpp"
DEF_LUAIMPORT(rev::IGLTexture)
