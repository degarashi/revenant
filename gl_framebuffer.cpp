#include "gl_framebuffer.hpp"
#include "gl_framebuffer_cmd.hpp"
#include "gl_if.hpp"
#include "gl_error.hpp"
#include "gl_texture.hpp"
#include "gl_renderbuffer.hpp"
#include "handler.hpp"

namespace rev {
	// ------------------------- GLFBufferCore -------------------------
	GLFBufferCore::GLFBufferCore(GLuint id):
		_idFbo(id)
	{}
	void GLFBufferCore::_attachRenderbuffer(const Att::e aId, const GLuint rb) {
		GLAssert(glFramebufferRenderbuffer, GL_FRAMEBUFFER, _AttIdtoGL(aId), GL_RENDERBUFFER, rb);
	}
	void GLFBufferCore::_attachCubeTexture(const Att::e aId, const GLuint faceFlag, const GLuint tb) {
		GL.glFramebufferTexture2D(GL_FRAMEBUFFER, _AttIdtoGL(aId), faceFlag, tb, 0);
	}
	void GLFBufferCore::_attachTexture(const Att::e aId, const GLuint tb) {
		_attachCubeTexture(aId, GL_TEXTURE_2D, tb);
	}
	void GLFBufferCore::use_begin() const {
		GL.glBindFramebuffer(GL_FRAMEBUFFER, _idFbo);
	}
	GLuint GLFBufferCore::getBufferId() const { return _idFbo; }

	// ------------------------- GLFBuffer -------------------------
	namespace {
		const auto fnReset = [](IGLResource* r) { r->onDeviceReset(); };
		// const auto fnLost = [](IGLResource* r) { r->onDeviceLost(); };

		template <class F>
		struct HdlVisitor : boost::static_visitor<> {
			F _f;
			HdlVisitor(F f): _f(f) {}
			void operator()(boost::blank) const {}
			void operator()(GLFBufferCore::RawTex& t) const {
				// 生のTextureIdは無効になる
				t = 0;
			}
			void operator()(GLFBufferCore::RawRb& r) const {
				// 生のRenderBufferIdは無効になる
				r = 0;
			}
			void operator()(GLFBufferCore::TexRes& t) const {
				(*this)(t.first);
			}
			template <class HDL>
			void operator()(HDL& hdl) const {
				_f(hdl.get());
			}
		};
		template <class F>
		HdlVisitor<F> MakeVisitor(F f) {
			return HdlVisitor<F>(f);
		}
	}
	GLFBuffer::GLFBuffer():
		GLFBufferCore(0)
	{}
	GLFBuffer::~GLFBuffer() {
		onDeviceLost();
	}
	void GLFBuffer::onDeviceReset() {
		if(_idFbo == 0) {
			GL.glGenFramebuffers(1, &_idFbo);
			GLAssert0();

			auto v = MakeVisitor(fnReset);
			for(auto& att : _attachment) {
				// Attachmentの復元は今行う (各ハンドルでの処理はif文で弾かれる)
				boost::apply_visitor(v, att);
			}
		}
	}
	namespace {
		using DstP = GLint (FBInfo::*);
		const std::pair<DstP, GLenum> c_dstp[] = {
			{&FBInfo::redSize, GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE},
			{&FBInfo::greenSize, GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE},
			{&FBInfo::blueSize, GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE},
			{&FBInfo::alphaSize, GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE},
			{&FBInfo::depthSize, GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE},
			{&FBInfo::stencilSize, GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE}
		};
	}
	FBInfo GLFBufferCore::GetCurrentInfo(const Att::e att) {
		FBInfo res;
		auto att_id = _AttIdtoGL(att);
		GLint ret;
		GL.glGetIntegerv(GL_FRAMEBUFFER_BINDING, &ret);
		if(ret == 0)
			att_id = GL_BACK_LEFT;
		GL.glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, att_id, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &ret);
		if(ret == GL_FRAMEBUFFER_DEFAULT) {
			if(att < Att::Depth) {
				att_id = GL_BACK_LEFT;
			} else {
				Assert0(att==Att::Depth);
				att_id = GL_DEPTH;
			}
		}
		for(auto& p : c_dstp) {
			GL.glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, att_id, p.second, &ret);
			res.*p.first = ret;
		}
		GL.glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, att_id, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &ret);
		res.id = ret;
		GL.glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, att_id, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &ret);
		res.bTex = ret == GL_TEXTURE;

		return res;
	}
	GLenum GLFBufferCore::_AttIdtoGL(const Att::e att) {
		const GLenum c_num[Att::NumAttachment] = {
			GL_COLOR_ATTACHMENT0,
			#ifndef USE_OPENGLES2
				GL_COLOR_ATTACHMENT1,
				GL_COLOR_ATTACHMENT2,
				GL_COLOR_ATTACHMENT3,
			#endif
			GL_DEPTH_ATTACHMENT,
			GL_STENCIL_ATTACHMENT
		};
		return c_num[int(att)];
	}
	void GLFBuffer::onDeviceLost() {
		if(_idFbo != 0) {
			GLW.getDrawHandler().postExecNoWait([buffId=getBufferId()](){
				D_GLWarn(glBindFramebuffer, GL_FRAMEBUFFER, buffId);
				for(int i=0 ; i<Att::NumAttachment ; i++) {
					// AttachmentのDetach
					D_GLWarn(glFramebufferRenderbuffer, GL_FRAMEBUFFER, _AttIdtoGL(Att::e(i)), GL_RENDERBUFFER, 0);
				}
				D_GLWarn(glBindFramebuffer, GL_FRAMEBUFFER, 0);
				D_GLWarn(glDeleteFramebuffers, 1, &buffId);
				GLAssert0();
			});
			_idFbo = 0;
			// Attachmentの解放は各ハンドルに任せる
		}
	}
	template <class T>
	void GLFBuffer::_attachIt(const Att::e att, const T& arg) {
		if(att == Att::DepthStencil) {
			// DepthとStencilそれぞれにhTexをセットする
			_attachIt(Att::Depth, arg);
			_attachIt(Att::Stencil, arg);
		} else
			_attachment[att] = arg;
	}
	void GLFBuffer::attachRBuffer(const Att::e att, const HRb& hRb) {
		_attachIt(att, HRb(hRb));
	}
	void GLFBuffer::attachTextureFace(const Att::e att, const HTexSrc& hTex, const CubeFace face) {
		_attachIt(att, TexRes(hTex, face));
	}
	void GLFBuffer::attachTexture(const Att::e att, const HTexSrc& hTex) {
		attachTextureFace(att, hTex, CubeFace::PositiveX);
	}
	void GLFBuffer::attachRawRBuffer(const Att::e att, const GLuint idRb) {
		_attachIt(att, RawRb(idRb));
	}
	void GLFBuffer::attachRawTexture(const Att::e att, const GLuint idTex) {
		_attachIt(att, RawTex(idTex));
	}
	void GLFBuffer::attachOtherAttachment(const Att::e attDst, const Att::e attSrc, const HFb& hFb) {
		_attachment[attDst] = hFb->getAttachment(attSrc);
	}
	void GLFBuffer::detach(const Att::e att) {
		_attachment[att] = boost::blank();
	}
	void GLFBuffer::dcmd_export(draw::IQueue& q) const {
		DCmd_Fb::Add(q, shared_from_this(), _attachment);
	}
	const GLFBuffer::Res& GLFBuffer::getAttachment(const Att::e att) const {
		return _attachment[att];
	}
	namespace {
		struct GetSize_Visitor : boost::static_visitor<Size_OP> {
			Size_OP operator()(boost::blank) const {
				return spi::none;
			}
			Size_OP operator()(const GLFBufferCore::TexRes& t) const {
				return (*this)(t.first);
			}
			template <class T>
			Size_OP operator()(const T& t) const {
				return t->getSize();
			}
			Size_OP operator()(const GLFBufferCore::RawTex& t) const {
				GLint id, w, h;
				GL.glGetIntegerv(GL_TEXTURE_BINDING_2D, &id);
				GL.glBindTexture(GL_TEXTURE_2D, t._value);
				GL.glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WIDTH, &w);
				GL.glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_HEIGHT, &h);
				GL.glBindTexture(GL_TEXTURE_2D, id);
				return lubee::SizeI(w,h);
			}
			Size_OP operator()(const GLFBufferCore::RawRb& t) const {
				GLint id, w, h;
				GL.glGetIntegerv(GL_RENDERBUFFER_BINDING, &id);
				GL.glBindRenderbuffer(GL_RENDERBUFFER, t._value);
				GL.glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &w);
				GL.glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &h);
				GL.glBindRenderbuffer(GL_RENDERBUFFER, id);
				return lubee::SizeI(w,h);
			}
		};
		struct GetTex_Visitor : boost::static_visitor<HTexSrc> {
			template <class T>
			HTexSrc operator()(const T&) const { return HTexSrc(); }
			HTexSrc operator()(const GLFBufferCore::TexRes& t) const { return t.first; }
		};
		struct GetRb_Visitor : boost::static_visitor<HRb> {
			template <class T>
			HRb operator()(const T&) const { return HRb(); }
			HRb operator()(const HRb& r) const { return r; }
		};
	}
	Size_OP GLFBuffer::GetAttachmentSize(const Res (&att)[Att::NumAttachment], const Att::e id) {
		return boost::apply_visitor(GetSize_Visitor(), att[id]);
	}
	Size_OP GLFBuffer::getAttachmentSize(const Att::e att) const {
		return GetAttachmentSize(_attachment, att);
	}
	const char* GLFBuffer::getResourceName() const noexcept {
		return "GLFBuffer";
	}
	HTexSrc GLFBuffer::getAttachmentAsTexture(const Att::e id) const {
		return boost::apply_visitor(GetTex_Visitor(), _attachment[id]);
	}
	HRb GLFBuffer::getAttachmentAsRBuffer(const Att::e id) const {
		return boost::apply_visitor(GetRb_Visitor(), _attachment[id]);
	}

	TLS<lubee::SizeI> GLFBufferCore::s_fbSize;
	void GLFBufferCore::_SetCurrentFBSize(const lubee::SizeI& s) {
		s_fbSize = s;
	}
	const lubee::SizeI& GLFBufferCore::GetCurrentFBSize() noexcept {
		return *s_fbSize;
	}
}
#include "lcv.hpp"
#include "lctable.hpp"
namespace rev {
	void GLFBuffer::LuaExport(LuaState& lsc) {
		auto tbl = std::make_shared<LCTable>();

		const auto reg = [&tbl](const Att flag) {
			(*tbl)[flag.toStr()] = lua_Integer(flag);
		};
		reg(Att::Color0);
		#ifndef USE_OPENGLES2
			reg(Att::Color1);
			reg(Att::Color2);
			reg(Att::Color3);
		#endif
		reg(Att::Depth);
		reg(Att::Stencil);
		reg(Att::DepthStencil);
		lsc.setField(-1, "Attribute", tbl);
	}
}
