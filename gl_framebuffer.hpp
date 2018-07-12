#pragma once
#include "gl_types.hpp"
#include "lubee/size.hpp"
#include "lubee/wrapper.hpp"
#include "sdl_tls.hpp"
#include "handle/opengl.hpp"
#include "lubee/rect.hpp"
#include <boost/variant.hpp>

namespace rev {
	namespace draw {
		class IQueue;
	}
	class GLFBufferCore {
		public:
			void use_begin() const;
			static TLS<lubee::SizeI> s_fbSize;

			DefineEnumPair(Att,
				((Color0)(0x00))
				#ifndef USE_OPENGLES2
					((Color1)(0x01))
					((Color2)(0x02))
					((Color3)(0x03))
				#endif
				((Depth)(0x04))
				((Stencil)(0x05))
				((NumAttachment)(0x06))
				#ifndef USE_OPENGLES2
					((DepthStencil)(0x07))
				#endif
			);
			static FBInfo GetCurrentInfo(Att::e att);
			static GLenum _AttIdtoGL(Att::e att);
			void _attachRenderbuffer(Att::e aId, GLuint rb);
			void _attachCubeTexture(Att::e aId, GLuint faceFlag, GLuint tb);
			void _attachTexture(Att::e aId, GLuint tb);
			using TexRes = std::pair<HTex, CubeFace>;
			struct RawTex : lubee::Wrapper<GLuint> {
				using Wrapper::Wrapper;
				using Wrapper::operator =;
			};
			struct RawRb : lubee::Wrapper<GLuint> {
				using Wrapper::Wrapper;
				using Wrapper::operator =;
			};
			// attachは受け付けるがハンドルを格納するだけであり、実際OpenGLにセットされるのはDrawThread
		protected:
			// 内部がTextureかRenderBufferなので、それらを格納できる型を定義
			using Res = boost::variant<boost::blank, RawTex, RawRb, TexRes, HRb>;
			GLuint	_idFbo;
			static void _SetCurrentFBSize(const lubee::SizeI& s);

		public:
			static const lubee::SizeI& GetCurrentFBSize() noexcept;
			GLFBufferCore() = default;
			GLFBufferCore(GLuint id);
			GLuint getBufferId() const;
	};
	struct DCmd_Fb : GLFBufferCore {
		// Pair::ResH
		struct Pair {
			bool	bTex;
			GLuint	resId,		// 0番は無効
					faceFlag;
		};

		Pair			ent[Att::NumAttachment];
		lubee::SizeI	size;	//!< Colo0バッファのサイズ

		using GLFBufferCore::GLFBufferCore;
		DCmd_Fb() = default;
		static void Add(draw::IQueue& q, const HFbC& fb, const Res (&att)[Att::NumAttachment]);
		static void AddTmp(draw::IQueue& q, const GLuint id);
		static void Command(const void* p);
	};
	//! 非ハンドル管理で一時的にFramebufferを使いたい時のヘルパークラス (内部用)
	class GLFBufferTmp : public GLFBufferCore {
		private:
			static void _Attach(GLenum flag, GLuint rb);
			const lubee::SizeI	_size;
		public:
			GLFBufferTmp(GLuint idFb, const lubee::SizeI& s);
			void attachRBuffer(Att::e att, GLuint rb);
			void attachTexture(Att::e att, GLuint id);
			void attachCubeTexture(Att::e att, GLuint id, GLuint face);

			void dcmd_export(draw::IQueue& q) const;
	};
	using Size_OP = spi::Optional<lubee::SizeI>;
	class LuaState;
	//! OpenGL: FrameBufferObjectインタフェース
	class GLFBuffer :
		public GLFBufferCore,
		public IGLResource,
		public std::enable_shared_from_this<GLFBuffer>
	{
		private:
			// GLuintは内部処理用 = RenderbufferのId
			Res	_attachment[Att::NumAttachment];
			template <class T>
			void _attachIt(Att::e att, const T& arg);

		public:
			static Size_OP GetAttachmentSize(const Res (&att)[Att::NumAttachment], Att::e id);
			static void LuaExport(LuaState& lsc);
			GLFBuffer();
			~GLFBuffer();
			void attachRBuffer(Att::e att, const HRb& hRb);
			void attachTexture(Att::e att, const HTex& hTex);
			void attachTextureFace(Att::e att, const HTex& hTex, CubeFace face);
			void attachRawRBuffer(Att::e att, GLuint idRb);
			void attachRawTexture(Att::e att, GLuint idTex);
			void attachOther(Att::e attDst, Att::e attSrc, HFb hFb);
			void detach(Att::e att);

			void onDeviceReset() override;
			void onDeviceLost() override;
			void dcmd_export(draw::IQueue& q) const;
			const Res& getAttachment(Att::e att) const;
			HTex getAttachmentAsTexture(Att::e id) const;
			HRb getAttachmentAsRBuffer(Att::e id) const;
			Size_OP getAttachmentSize(Att::e att) const;
			const char* getResourceName() const noexcept override;
	};
}
#include "luaimport.hpp"
DEF_LUAIMPORT(rev::GLFBuffer)
