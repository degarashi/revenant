#include "gl_framebuffer.hpp"
#include "drawcmd/queue_if.hpp"
#include "systeminfo.hpp"
#include "gl_texture.hpp"
#include "gl_renderbuffer.hpp"
#include "gl_if.hpp"
#include "gl_error.hpp"

namespace rev {
	namespace {
		struct Visitor : boost::static_visitor<> {
			DCmd_Fb::Pair&		_dst;
			draw::IQueue&		_q;
			Visitor(DCmd_Fb::Pair& dst, draw::IQueue& q):
				_dst(dst),
				_q(q)
			{}

			void operator()(const DCmd_Fb::RawRb& r) const {
				_dst.bTex = false;
				_dst.resId = r;
			}
			void operator()(const DCmd_Fb::RawTex& t) const {
				_dst.bTex = true;
				_dst.faceFlag = GL_TEXTURE_2D;
				_dst.resId = t;
			}
			void operator()(const DCmd_Fb::TexRes& t) const {
				auto* tr = t.first.get();
				_dst.bTex = true;
				_dst.faceFlag = tr->getFaceFlag(t.second);
				_q.stockResource(t.first);
				_dst.resId = tr->getTextureId();
			}
			void operator()(const HRb& hlRb) const {
				_dst.bTex = false;
				_q.stockResource(hlRb);
				_dst.faceFlag = 0;
				_dst.resId = hlRb->getBufferId();
			}
			void operator()(boost::blank) const {
				_dst.resId = 0;
			}
		};
	}
	void DCmd_Fb::Add(draw::IQueue& q, const HFbC& fb, const Res (&att)[Att::NUM_ATTACHMENT]) {
		q.stockResource(fb);
		DCmd_Fb fbc(fb->getBufferId());
		fbc.size = *GLFBuffer::GetAttachmentSize(att, Att::COLOR0);
		for(std::size_t i=0 ; i<Att::NUM_ATTACHMENT ; i++)
			boost::apply_visitor(Visitor(fbc.ent[i], q), att[i]);
		q.add(fbc);
	}
	void DCmd_Fb::AddTmp(draw::IQueue& q, const GLuint id) {
		DCmd_Fb fb(id);
		fb.size = mgr_info.getScreenSize();
		for(auto& e : fb.ent)
			e.resId = 0;
		q.add(fb);
	}
	void DCmd_Fb::Command(const void* p) {
		auto& self = *static_cast<const DCmd_Fb*>(p);
		self.use_begin();
		for(std::size_t i=0 ; i<Att::NUM_ATTACHMENT ; i++) {
			auto& p = self.ent[i];
			if(p.resId != 0) {
				const auto flag = _AttIdtoGL(Att::Id(i));
				if(p.bTex)
					GLAssert(glFramebufferTexture2D, GL_FRAMEBUFFER, flag, p.faceFlag, p.resId, 0);
				else
					GLAssert(glFramebufferRenderbuffer, GL_FRAMEBUFFER, flag, GL_RENDERBUFFER, p.resId);
			}
		}
		// この時点で有効なフレームバッファになって無ければエラー
		const GLenum e = GL.glCheckFramebufferStatus(GL_FRAMEBUFFER);
		Assert(e==GL_FRAMEBUFFER_COMPLETE, GLFormat::QueryEnumString(e).c_str());

		// 後で参照するためにFramebuff(Color0)のサイズを記録
		GLFBufferCore::_SetCurrentFBSize(self.size);
	}
}
