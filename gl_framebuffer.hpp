#pragma once
#include "gl_types.hpp"
#include "lubee/size.hpp"
#include "sdl_tls.hpp"
#include "handle/opengl.hpp"
#include "lubee/rect.hpp"
#include <boost/variant.hpp>

namespace rev {
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
			using TexRes = std::pair<HTexSrc, CubeFace>;

			struct RawTex {
				GLuint	id;

				void invalidate() noexcept;
				bool operator == (const RawTex& t) const noexcept;
			};
			struct RawRb {
				GLuint	id;

				void invalidate() noexcept;
				bool operator == (const RawRb& r) const noexcept;
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
	namespace draw {
		class IQueue;
	}
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
			void attachTexture(Att::e att, const HTexSrc& hTex);
			void attachTextureFace(Att::e att, const HTexSrc& hTex, CubeFace face);
			void attachRawRBuffer(Att::e att, GLuint idRb);
			void attachRawTexture(Att::e att, GLuint idTex);
			void attachOtherAttachment(Att::e attDst, Att::e attSrc, const HFb& hFb);
			void detach(Att::e att);

			void onDeviceReset() override;
			void onDeviceLost() override;
			void dcmd_export(draw::IQueue& q) const;
			const Res& getAttachment(Att::e att) const;
			HTexSrc getAttachmentAsTexture(Att::e id) const;
			HRb getAttachmentAsRBuffer(Att::e id) const;
			Size_OP getAttachmentSize(Att::e att) const;
			const char* getResourceName() const noexcept override;
	};
}
#include "luaimport.hpp"
DEF_LUAIMPORT(rev::GLFBuffer)
