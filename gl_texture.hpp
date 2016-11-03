#pragma once
#include "gl_types.hpp"
#include "gl_format.hpp"
#include "lubee/size.hpp"
#include "lubee/rect.hpp"
#include "drawtoken/drawtoken_t.hpp"
#include "abstbuffer.hpp"
#include "uri.hpp"
#include "chunk.hpp"

namespace rev {
	class Surface;
	using Surface_SP = std::shared_ptr<Surface>;
	class PathBlock;
	using ByteBuff = std::vector<uint8_t>;
	namespace draw {
		class Texture;
		class TextureA;
	}
	//! OpenGLテクスチャインタフェース
	/*!	フィルターはNEARESTとLINEARしか無いからboolで管理 */
	class IGLTexture : public IGLResource, public std::enable_shared_from_this<IGLTexture> {
		public:
			friend class RUser<IGLTexture>;
			friend class draw::Texture;
			friend class draw::TextureA;

			void use_begin() const;
			void use_end() const;
		protected:
			GLuint		_idTex;
			int			_iLinearMag,	//!< Linearの場合は1, Nearestは0
						_iLinearMin;
			WrapState	_wrapS,
						_wrapT;
			GLuint		_actId;		//!< セットしようとしているActiveTextureId (for Use())
			//! [mipLevel][Nearest / Linear]
			const static GLuint cs_Filter[3][2];
			const MipState		_mipLevel;
			GLuint				_texFlag,	//!< TEXTURE_2D or TEXTURE_CUBE_MAP
								_faceFlag;	//!< TEXTURE_2D or TEXTURE_CUBE_MAP_POSITIVE_X
			float				_coeff;
			lubee::SizeI		_size;
			InCompressedFmt_OP	_format;	//!< 値が無効 = 不定

			bool _onDeviceReset();
			IGLTexture(MipState miplevel, InCompressedFmt_OP fmt, const lubee::SizeI& sz, bool bCube);
			IGLTexture(const IGLTexture& t);

		public:
			IGLTexture(IGLTexture&& t);
			virtual ~IGLTexture();

			void setFilter(bool bLinearMag, bool bLinearMin);
			void setLinear(bool bLinear);
			void setAnisotropicCoeff(float coeff);
			void setUVWrap(WrapState s, WrapState t);
			void setWrap(WrapState st);
			const char* getResourceName() const noexcept override;

			RUser<IGLTexture> use() const;

			const lubee::SizeI& getSize() const;
			GLint getTextureId() const;
			const InCompressedFmt_OP& getFormat() const;
			GLenum getTexFlag() const;
			GLenum getFaceFlag(CubeFace face=CubeFace::PositiveX) const;
			void onDeviceLost() override;
			//! テクスチャユニット番号を指定してBind
			void setActiveId(GLuint n);

			static bool IsMipmap(MipState level);
			bool isMipmap() const;
			//! 内容をファイルに保存 (主にデバッグ用)
			void save(const PathBlock& path, CubeFace face=CubeFace::PositiveX);

			bool isCubemap() const;
			bool operator == (const IGLTexture& t) const;
			ByteBuff readData(GLInFmt internalFmt, GLTypeFmt elem, int level=0, CubeFace face=CubeFace::PositiveX) const;
			ByteBuff readRect(GLInFmt internalFmt, GLTypeFmt elem, const lubee::RectI& rect, CubeFace face=CubeFace::PositiveX) const;
			/*! \param[in] uniform変数の番号
				\param[in] index idで示されるuniform変数配列のインデックス(デフォルト=0)
				\param[in] hRes 自身のリソースハンドル */
			void getDrawToken(draw::TokenDst& dst, GLint id, int index, int actId);
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
	};
	std::pair<lubee::SizeI,GLInCompressedFmt> MakeTex(GLenum tflag, const Surface_SP& sfc, InCompressedFmt_OP fmt, bool bP2, bool bMip);
	std::pair<lubee::SizeI,GLInCompressedFmt> MakeMip(GLenum tflag, GLenum format, const lubee::SizeI& size, const ByteBuff& buff, bool bP2, bool bMip);

	//! 画像ファイルから2Dテクスチャを読む
	/*! DeviceReset時:
		再度ファイルから読み出す */
	class Texture_StaticURI : public IGLTexture {
		private:
			URI					_uri;
			InCompressedFmt_OP	_opFmt;
		public:
			static std::pair<lubee::SizeI, GLInCompressedFmt> LoadTexture(IGLTexture& tex, HRW hRW, CubeFace face);
			Texture_StaticURI(Texture_StaticURI&& t);
			Texture_StaticURI(const URI& uri, MipState miplevel, InCompressedFmt_OP fmt);
			void onDeviceReset() override;
	};

	template <class T>
	struct IOPArray {
		virtual int getNPacked() const = 0;
		virtual const T& getPacked(int n) const = 0;
		virtual ~IOPArray() {}
		virtual uint32_t getID() const = 0;
		virtual bool operator == (const IOPArray& p) const = 0;
	};
	template <class T, int N>
	class OPArray : public IOPArray<T> {
		private:
			spi::Optional<T>	_packed[N];

			void _init(spi::Optional<T>* /*dst*/) {}
			template <class TA, class... Ts>
			void _init(spi::Optional<T>* dst, TA&& ta, Ts&&... ts) {
				*dst++ = std::forward<TA>(ta);
				_init(dst, std::forward<Ts>(ts)...);
			}
		public:
			template <class... Ts>
			OPArray(Ts&&... ts) {
				static_assert(sizeof...(Ts)==N, "invalid number of argument(s)");
				_init(_packed, std::forward<Ts>(ts)...);
			}
			int getNPacked() const override {
				return N;
			}
			const T& getPacked(const int n) const override {
				Assert0(n<=N);
				return *_packed[n];
			}
			bool operator == (const IOPArray<T>& p) const override {
				if(getID()==p.getID() && getNPacked()==p.getNPacked()) {
					auto& p2 = reinterpret_cast<const OPArray&>(p);
					for(int i=0 ; i<N ; i++) {
						if(_packed[i] != p2._packed[i])
							return false;
					}
					return true;
				}
				return false;
			}
			uint32_t getID() const override {
				return MakeChunk('P','a','c','k');
			}
	};
	using SPPackURI = std::shared_ptr<IOPArray<URI>>;
	//! 連番または6つの画像ファイルからCubeテクスチャを読む
	class Texture_StaticCubeURI : public IGLTexture {
		private:
			SPPackURI			_uri;
			InCompressedFmt_OP	_opFmt;
		public:
			Texture_StaticCubeURI(Texture_StaticCubeURI&& t);
			Texture_StaticCubeURI(const URI& uri, MipState miplevel, InCompressedFmt_OP fmt);
			Texture_StaticCubeURI(const URI& uri0, const URI& uri1, const URI& uri2,
				const URI& uri3, const URI& uri4, const URI& uri5, MipState miplevel, InCompressedFmt_OP fmt);
			void onDeviceReset() override;
	};
}
