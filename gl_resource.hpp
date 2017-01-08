//! GLリソース管理
#pragma once
#include "gl_types.hpp"
#include "gl_format.hpp"
#include "resmgr_app.hpp"
#include "lubee/size.hpp"
#include "resmgr_base.hpp"

namespace rev {
	class GLFBufferTmp;
	#define mgr_gl (::rev::GLRes::ref())
	//! OpenGL関連のリソースマネージャ
	class GLRes : public ResMgrApp<IGLResource>, public ResMgrBase, public spi::Singleton<GLRes> {
		private:
			using base_type = ResMgrApp<IGLResource>;

			using Fb_UP = std::unique_ptr<GLFBuffer>;
			Fb_UP							_upFb;
			std::unique_ptr<GLFBufferTmp>	_tmpFb;
			FBInfo_OP						_defaultDepth,
											_defaultColor;
			//! 空のテクスチャ (何もテクスチャをセットしない事を示す)
			/*! デバッグで色を変えたりしてチェックできる */
			HTex							_hEmptyTex;
			//! DeviceLost/Resetの状態管理
			bool	_bInit;
			//! デストラクタ内の時はtrue
			bool	_bInDtor;

			DefineEnum(
				ResourceType,
				(Texture)
				(Effect)
			);
			const static std::string cs_rtname[ResourceType::_Num];
			void _resourceInit(IGLResource* r);

			//! キューブマップの区別のためのポストフィックス
			spi::Optional<char>	_chPostfix;
			void _modifyResourceName(URI& key) const override;
			void _initDefaultInfo();
			void _clearDefaultInfo();

		public:
			GLRes();
			~GLRes();
			bool deviceStatus() const;
			bool isInDtor() const;
			void onDeviceLost();
			void onDeviceReset();

			//! ベースクラスのacquireメソッドを隠す為のダミー
			void acquire();

			// static void LuaExport(LuaState& lsc);
			// ------------ Texture ------------
			//! ファイルからテクスチャを読み込む
			/*!
				圧縮テクスチャはファイルヘッダで判定
				\param[in] fmt OpenGLの内部フォーマット(not ファイルのフォーマット)<br>
								指定しなければファイルから推定
			*/
			HTex loadTextureUri(const URI& uri, MipState miplevel=MipState::NoMipmap, InCompressedFmt_OP fmt=spi::none);
			HTex loadTexture(const std::string& name, MipState miplevel=MipState::NoMipmap, InCompressedFmt_OP fmt=spi::none);
			//! 連番ファイルからキューブテクスチャを作成
			HTex loadCubeTextureUri(const URI& uri, MipState miplevel=MipState::NoMipmap, InCompressedFmt_OP fmt=spi::none);
			HTex loadCubeTexture(const std::string& name, MipState miplevel=MipState::NoMipmap, InCompressedFmt_OP fmt=spi::none);
			//! 個別のファイルからキューブテクスチャを作成
			/*! 画像サイズとフォーマットは全て一致していなければならない */
			HTex _loadCubeTexture(MipState miplevel, InCompressedFmt_OP fmt, const URI& uri0, const URI& uri1, const URI& uri2,
								  const URI& uri3, const URI& uri4, const URI& uri5);
			template <class... Ts>
			HTex loadCubeTextureFromResource(MipState miplevel, InCompressedFmt_OP fmt, Ts&&... ts) {
				return loadCubeTexture(miplevel, fmt, _uriFromResourceName(std::forward<Ts>(ts))...);
			}
			HTex _createTexture(bool bCube, const lubee::SizeI& size, GLInSizedFmt fmt, bool bStream, bool bRestore);
			//! 空のテクスチャを作成
			/*! 領域だけ確保 */
			HTex createTexture(const lubee::SizeI& size, GLInSizedFmt fmt, bool bStream, bool bRestore);
			/*! 用意したデータで初期化 */
			HTex createTextureInit(const lubee::SizeI& size, GLInSizedFmt fmt, bool bStream, bool bRestore, GLTypeFmt srcFmt, AB_Byte data);
			//! 空のキューブテクスチャを作成
			HTex createCubeTexture(const lubee::SizeI& size, GLInSizedFmt fmt, bool bRestore, bool bStream);
			//! 共通のデータで初期化
			HTex createCubeTextureInit(const lubee::SizeI& size, GLInSizedFmt fmt, bool bRestore, bool bStream, AB_Byte data);
			//! 個別のデータで初期化
			HTex createCubeTextureInit(const lubee::SizeI& size, GLInSizedFmt fmt, bool bRestore, bool bStream,
									AB_Byte data0, AB_Byte data1, AB_Byte data2,
									AB_Byte data3, AB_Byte data4, AB_Byte data5);

			// ------------ Shader ------------
			//! 文字列からシェーダーを作成
			HSh makeShader(ShType type, const std::string& src);

			//! 複数のシェーダーからプログラムを作成 (vertex, geometry, fragment)
			HProg makeProgram(const HSh& vsh, const HSh& psh);
			//! 複数のシェーダーからプログラムを作成 (vertex, fragment)
			HProg makeProgram(const HSh& vsh, const HSh& gsh, const HSh& psh);

			// ------------ Buffer ------------
			//! ファイルからエフェクトの読み込み
			template <class T>
			HFx loadEffect(const std::string& name) {
				_chPostfix = spi::none;
				_setResourceTypeId(ResourceType::Effect);
				auto h = base_type::acquireA<T>(name);
				_resourceInit(h.get());
				return h;
			}
			//! 頂点バッファの確保
			HVb makeVBuffer(DrawType dtype);
			//! インデックスバッファの確保
			HIb makeIBuffer(DrawType dtype);

			HTex getEmptyTexture() const;
			HGLRes _common(const std::string& key, std::function<HGLRes (const URI&)> cb);
			GLFBufferTmp& getTmpFramebuffer() const;
			// --- from ResMgrBase ---
			HRes loadResource(const URI& uri) override;

			// ------------ FrameBuffer ------------
			HFb makeFBuffer();
			// ------------ RenderBuffer ------------
			HRb makeRBuffer(int w, int h, GLInRenderFmt fmt);

			const FBInfo_OP& getDefaultDepth() const;
			const FBInfo_OP& getDefaultColor() const;
	};
}
