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
			friend class RUser<GLFBufferCore>;
			void use_begin() const;
			void use_end() const;
			static TLS<lubee::SizeI> s_fbSize;

			struct Att {
				enum Id {
					COLOR0,
					#ifndef USE_OPENGLES2
						COLOR1,
						COLOR2,
						COLOR3,
					#endif
					DEPTH,
					STENCIL,
					NUM_ATTACHMENT,
					DEPTH_STENCIL = 0xffff
				};
			};
			static FBInfo GetCurrentInfo(Att::Id att);
			static GLenum _AttIdtoGL(Att::Id att);
			void _attachRenderbuffer(Att::Id aId, GLuint rb);
			void _attachCubeTexture(Att::Id aId, GLuint faceFlag, GLuint tb);
			void _attachTexture(Att::Id aId, GLuint tb);
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

		Pair			ent[Att::NUM_ATTACHMENT];
		lubee::SizeI	size;	//!< Colo0バッファのサイズ

		using GLFBufferCore::GLFBufferCore;
		DCmd_Fb() = default;
		static void Add(draw::IQueue& q, const HFbC& fb, const Res (&att)[Att::NUM_ATTACHMENT]);
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
			void attachRBuffer(Att::Id att, GLuint rb);
			void attachTexture(Att::Id att, GLuint id);
			void attachCubeTexture(Att::Id att, GLuint id, GLuint face);
			void use_end() const;

			void dcmd_fb(draw::IQueue& q) const;
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
			Res	_attachment[Att::NUM_ATTACHMENT];
			template <class T>
			void _attachIt(Att::Id att, const T& arg);

		public:
			static Size_OP GetAttachmentSize(const Res (&att)[Att::NUM_ATTACHMENT], Att::Id id);
			static void LuaExport(LuaState& lsc);
			GLFBuffer();
			~GLFBuffer();
			void attachRBuffer(Att::Id att, HRb hRb);
			void attachTexture(Att::Id att, HTex hTex);
			void attachTextureFace(Att::Id att, HTex hTex, CubeFace face);
			void attachRawRBuffer(Att::Id att, GLuint idRb);
			void attachRawTexture(Att::Id att, GLuint idTex);
			void attachOther(Att::Id attDst, Att::Id attSrc, HFb hFb);
			void detach(Att::Id att);

			void onDeviceReset() override;
			void onDeviceLost() override;
			void dcmd_fb(draw::IQueue& q) const;
			const Res& getAttachment(Att::Id att) const;
			HTex getAttachmentAsTexture(Att::Id id) const;
			HRb getAttachmentAsRBuffer(Att::Id id) const;
			Size_OP getAttachmentSize(Att::Id att) const;
			const char* getResourceName() const noexcept override;
	};
}
#include "luaimport.hpp"
DEF_LUAIMPORT(rev::GLFBuffer)
