//! GLリソース管理
#pragma once
#include "types.hpp"
#include "format.hpp"
#include "../resmgr_app.hpp"
#include "../resmgr_base.hpp"
#include "../handle/opengl.hpp"
#include "lubee/src/size.hpp"

namespace rev {
	class GLFBuffer;
	class GLFBufferTmp;
	class LuaState;
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
			);
			const static std::string cs_rtname[ResourceType::_Num];
			void _resourceInit(IGLResource* r);
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

			static void LuaExport(LuaState& lsc);
			// ------------ Texture ------------
			HTexF createTexFilter();
			HTex attachTexFilter(const HTexSrcC& src, const HTexF& filter);
			//! ファイルからテクスチャを読み込む
			/*!
				圧縮テクスチャはファイルヘッダで判定
				\param[in] fmt OpenGLの内部フォーマット(not ファイルのフォーマット)<br>
								指定しなければファイルから推定
			*/
			HTexSrc loadTexture(const URI& uri, MipState miplevel=MipState::NoMipmap, InCompressedFmt_OP fmt=spi::none);
			//! 個別のファイルからキューブテクスチャを作成
			/*! 画像サイズとフォーマットは全て一致していなければならない */
			HTexSrc loadCubeTexture(MipState miplevel, InCompressedFmt_OP fmt,
								const URI& uri0, const URI& uri1, const URI& uri2,
								const URI& uri3, const URI& uri4, const URI& uri5);
			HTexMem2D loadTextureFromRW(const HRW& rw);
			//! 空のテクスチャを作成
			/*! 領域だけ確保 */
			HTexMem2D createTexture(const lubee::SizeI& size, GLInSizedFmt fmt, bool mip, bool bRestore);

			/*! 用意したデータで初期化 */
			HTexMem2D createTextureInit(const lubee::SizeI& size, GLInSizedFmt fmt, bool mip, bool bRestore, GLTypeFmt elem, AB_Byte data);
			//! 空のキューブテクスチャを作成
			HTexMemCube createCubeTexture(const lubee::SizeI& size, GLInSizedFmt fmt, bool mip, bool bRestore);

			// ------------ Shader ------------
			//! 文字列からシェーダーを作成
			HSh makeShader(ShType type, const std::string& src);

			//! 複数のシェーダーからプログラムを作成 (vertex, geometry, fragment)
			HProg makeProgram(const HSh& vsh, const HSh& psh);
			//! 複数のシェーダーからプログラムを作成 (vertex, fragment)
			HProg makeProgram(const HSh& vsh, const HSh& gsh, const HSh& psh);

			template <class T, class... Ts>
			std::shared_ptr<T> makeResource(Ts&&... ts) {
				auto h = base_type::emplaceA_WithType<T>(std::forward<Ts>(ts)...);
				_resourceInit(h.get());
				return h;
			}
			// ------------ Buffer ------------
			//! 頂点バッファの確保
			HVb makeVBuffer(DrawType dtype);
			//! インデックスバッファの確保
			HIb makeIBuffer(DrawType dtype);

			HTex getEmptyTexture() const;
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
#include "../lua/import.hpp"
DEF_LUAIMPORT(rev::GLRes)
