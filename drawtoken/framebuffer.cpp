#include "framebuffer.hpp"
#include "../gl_texture.hpp"
#include "../gl_renderbuffer.hpp"
#include "../gl_error.hpp"
#include "../gl_if.hpp"

namespace rev {
	namespace draw {
		// ------------------------- draw::FrameBuff -------------------------
		struct FrameBuff::Visitor : boost::static_visitor<> {
			draw::FrameBuff::Pair& _dst;
			Visitor(draw::FrameBuff::Pair& dst): _dst(dst) {}

			void operator()(const RawRb& r) const {
				_dst.bTex = false;
				_dst.idRes = r;
			}
			void operator()(const RawTex& t) const {
				_dst.bTex = true;
				_dst.faceFlag = GL_TEXTURE_2D;
				_dst.idRes = t;
			}
			void operator()(const TexRes& t) const {
				auto* tr = t.first.get();
				_dst.bTex = true;
				_dst.faceFlag = tr->getFaceFlag(t.second);
				_dst.handle = t;
				_dst.idRes = tr->getTextureId();
			}
			void operator()(const HRb& hlRb) const {
				_dst.bTex = false;
				_dst.handle = hlRb;
				_dst.faceFlag = 0;
				_dst.idRes = hlRb->getBufferId();
			}
			void operator()(boost::blank) const {
				_dst.idRes = 0;
			}
		};
		FrameBuff::FrameBuff(const GLuint idFb, const lubee::SizeI& s):
			GLFBufferCore(idFb),
			_size(s)
		{
			for(auto& p : _ent)
				p.idRes = 0;
		}
		FrameBuff::FrameBuff(const HFb& hFb, const Res (&att)[Att::NUM_ATTACHMENT]):
			GLFBufferCore(hFb->getBufferId()),
			_hFb(hFb),
			_size(*GLFBuffer::GetAttachmentSize(att, Att::COLOR0))
		{
			for(int i=0 ; i<Att::NUM_ATTACHMENT ; i++)
				boost::apply_visitor(Visitor(_ent[i]), att[i]);
		}
		void FrameBuff::exec() {
			use_begin();
			for(int i=0 ; i<Att::NUM_ATTACHMENT ; i++) {
				auto& p = _ent[i];
				if(p.idRes != 0) {
					auto flag = _AttIdtoGL(Att::Id(i));
					if(p.bTex)
						GLAssert(glFramebufferTexture2D, GL_FRAMEBUFFER, flag, p.faceFlag, p.idRes, 0);
					else
						GLAssert(glFramebufferRenderbuffer, GL_FRAMEBUFFER, flag, GL_RENDERBUFFER, p.idRes);
				}
			}
			// この時点で有効なフレームバッファになって無ければエラー
			GLenum e = GL.glCheckFramebufferStatus(GL_FRAMEBUFFER);
			Assert(e==GL_FRAMEBUFFER_COMPLETE, GLFormat::QueryEnumString(e).c_str());

			// 後で参照するためにFramebuff(Color0)のサイズを記録
			GLFBufferCore::_SetCurrentFBSize(_size);
		}
	}
}
