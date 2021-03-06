#include "framebuffer_cmd.hpp"
#include "../drawcmd/queue_if.hpp"
#include "systeminfo.hpp"
#include "texture.hpp"
#include "renderbuffer.hpp"
#include "if.hpp"
#include "error.hpp"

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
				_dst.resId = r.id;
			}
			void operator()(const DCmd_Fb::RawTex& t) const {
				_dst.bTex = true;
				_dst.faceFlag = GL_TEXTURE_2D;
				_dst.resId = t.id;
			}
			void operator()(const DCmd_Fb::TexRes& t) const {
				auto* tr = t.tex.get();
				_dst.bTex = true;
				_dst.faceFlag = tr->getFaceFlag(t.face);
				_dst.resId = tr->getTextureId();
				_dst.level = t.level;
				_q.stockResource(t.tex);
			}
			void operator()(const HRb& r) const {
				_dst.bTex = false;
				_dst.faceFlag = 0;
				_dst.resId = r->getBufferId();
				_q.stockResource(r);
			}
			void operator()(boost::blank) const {
				_dst.resId = 0;
			}
		};
	}
	void DCmd_Fb::MakeCommand_SetAttachment(draw::IQueue& q, const HFbC& fb, const Res (&att)[Att::NumAttachment]) {
		q.stockResource(fb);
		DCmd_Fb fbc(fb->getBufferId());
		fbc.size = *GLFBuffer::GetAttachmentSize(att, Att::Color0);
		for(std::size_t i=0 ; i<Att::NumAttachment ; i++)
			boost::apply_visitor(Visitor(fbc.ent[i], q), att[i]);
		q.add(fbc);
	}
	void DCmd_Fb::MakeCommand_ApplyOnly(draw::IQueue& q, const GLuint fbId) {
		DCmd_Fb fb(fbId);
		fb.size = mgr_info.getScreenSize();
		for(auto& e : fb.ent)
			e.resId = 0;
		q.add(fb);
	}
	void DCmd_Fb::Command(const void* p) {
		auto& self = *static_cast<const DCmd_Fb*>(p);
		self.use_begin();
		for(std::size_t i=0 ; i<Att::NumAttachment ; i++) {
			auto& p = self.ent[i];
			if(p.resId != 0) {
				const auto flag = _AttIdtoGL(Att::e(i));
				if(p.bTex)
					GLAssert(glFramebufferTexture2D, GL_FRAMEBUFFER, flag, p.faceFlag, p.resId, p.level);
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
