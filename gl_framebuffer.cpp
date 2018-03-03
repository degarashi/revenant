#include "gl_framebuffer.hpp"
#include "gl_if.hpp"
#include "gl_error.hpp"
#include "gl_texture.hpp"
#include "gl_renderbuffer.hpp"
#include "systeminfo.hpp"
#include "handler.hpp"
#include "drawtoken/framebuffer.hpp"

namespace rev {
	// ------------------------- GLFBufferTmp -------------------------
	GLFBufferTmp::GLFBufferTmp(const GLuint idFb, const lubee::SizeI& s):
		GLFBufferCore(idFb),
		_size(s)
	{}
	RUser<GLFBufferTmp> GLFBufferTmp::use() const {
		return RUser<GLFBufferTmp>(*this);
	}
	void GLFBufferTmp::use_end() const {
		for(int i=0 ; i<Att::NUM_ATTACHMENT ; i++)
			const_cast<GLFBufferTmp*>(this)->_attachRenderbuffer(Att::Id(i), 0);
	}
	void GLFBufferTmp::attachRBuffer(const Att::Id att, const GLuint rb) {
		#ifdef USE_OPENGLES2
			D_Assert0(att != Att::DEPTH_STENCIL);
		#endif
		_attachRenderbuffer(att, rb);
	}
	void GLFBufferTmp::attachTexture(const Att::Id att, const GLuint id) {
		#ifdef USE_OPENGLES2
			D_Assert0(att != Att::DEPTH_STENCIL);
		#endif
		_attachTexture(att, id);
	}
	void GLFBufferTmp::attachCubeTexture(const Att::Id att, const GLuint id, const GLuint face) {
		#ifdef USE_OPENGLES2
			D_Assert0(att != Att::DEPTH_STENCIL);
		#endif
		_attachCubeTexture(att, face, id);
	}
	void GLFBufferTmp::getDrawToken(draw::TokenDst& dst) const {
		using UT = draw::FrameBuff;
		new(dst.allocate_memory(sizeof(UT), draw::CalcTokenOffset<UT>())) UT(_idFbo, mgr_info.getScreenSize());
	}

	// ------------------------- GLFBufferCore -------------------------
	GLFBufferCore::GLFBufferCore(GLuint id):
		_idFbo(id)
	{}
	RUser<GLFBufferCore> GLFBufferCore::use() const {
		return RUser<GLFBufferCore>(*this);
	}
	void GLFBufferCore::_attachRenderbuffer(const Att::Id aId, const GLuint rb) {
		GLAssert(glFramebufferRenderbuffer, GL_FRAMEBUFFER, _AttIdtoGL(aId), GL_RENDERBUFFER, rb);
	}
	void GLFBufferCore::_attachCubeTexture(const Att::Id aId, const GLuint faceFlag, const GLuint tb) {
		GL.glFramebufferTexture2D(GL_FRAMEBUFFER, _AttIdtoGL(aId), faceFlag, tb, 0);
	}
	void GLFBufferCore::_attachTexture(const Att::Id aId, const GLuint tb) {
		_attachCubeTexture(aId, GL_TEXTURE_2D, tb);
	}
	void GLFBufferCore::use_begin() const {
		GL.glBindFramebuffer(GL_FRAMEBUFFER, _idFbo);
	}
	void GLFBufferCore::use_end() const {
		GL.glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
	FBInfo GLFBufferCore::GetCurrentInfo(Att::Id att) {
		FBInfo res;
		auto att_id = _AttIdtoGL(att);
		GLint ret;
		GL.glGetIntegerv(GL_FRAMEBUFFER_BINDING, &ret);
		if(ret == 0)
			att_id = GL_BACK_LEFT;
		GL.glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, att_id, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &ret);
		if(ret == GL_FRAMEBUFFER_DEFAULT) {
			if(att < Att::DEPTH) {
				att_id = GL_BACK_LEFT;
			} else {
				Assert0(att==Att::DEPTH);
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
	GLenum GLFBufferCore::_AttIdtoGL(Att::Id att) {
		const GLenum c_num[Att::NUM_ATTACHMENT] = {
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
				for(int i=0 ; i<Att::NUM_ATTACHMENT ; i++) {
					// AttachmentのDetach
					D_GLWarn(glFramebufferRenderbuffer, GL_FRAMEBUFFER, _AttIdtoGL(Att::Id(i)), GL_RENDERBUFFER, 0);
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
	void GLFBuffer::_attachIt(Att::Id att, const T& arg) {
		if(att == Att::DEPTH_STENCIL) {
			// DepthとStencilそれぞれにhTexをセットする
			_attachIt(Att::DEPTH, arg);
			_attachIt(Att::STENCIL, arg);
		} else
			_attachment[att] = arg;
	}
	void GLFBuffer::attachRBuffer(Att::Id att, HRb hRb) {
		_attachIt(att, HRb(hRb));
	}
	void GLFBuffer::attachTextureFace(Att::Id att, HTex hTex, const CubeFace face) {
		_attachIt(att, TexRes(hTex, face));
	}
	void GLFBuffer::attachTexture(Att::Id att, HTex hTex) {
		attachTextureFace(att, hTex, CubeFace::PositiveX);
	}
	void GLFBuffer::attachRawRBuffer(Att::Id att, GLuint idRb) {
		_attachIt(att, RawRb(idRb));
	}
	void GLFBuffer::attachRawTexture(Att::Id att, GLuint idTex) {
		_attachIt(att, RawTex(idTex));
	}
	void GLFBuffer::attachOther(Att::Id attDst, Att::Id attSrc, HFb hFb) {
		_attachment[attDst] = hFb->getAttachment(attSrc);
	}
	void GLFBuffer::detach(Att::Id att) {
		_attachment[att] = boost::blank();
	}
	void GLFBuffer::getDrawToken(draw::TokenDst& dst) const {
		using UT = draw::FrameBuff;
		new(dst.allocate_memory(sizeof(UT), draw::CalcTokenOffset<UT>()))
			UT(const_cast<GLFBuffer*>(this)->shared_from_this(), _attachment);
	}
	const GLFBuffer::Res& GLFBuffer::getAttachment(Att::Id att) const {
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
		struct GetTex_Visitor : boost::static_visitor<HTex> {
			template <class T>
			HTex operator()(const T&) const { return HTex(); }
			HTex operator()(const GLFBufferCore::TexRes& t) const { return t.first; }
		};
		struct GetRb_Visitor : boost::static_visitor<HRb> {
			template <class T>
			HRb operator()(const T&) const { return HRb(); }
			HRb operator()(const HRb& r) const { return r; }
		};
	}
	Size_OP GLFBuffer::GetAttachmentSize(const Res (&att)[Att::NUM_ATTACHMENT], Att::Id id) {
		return boost::apply_visitor(GetSize_Visitor(), att[id]);
	}
	Size_OP GLFBuffer::getAttachmentSize(Att::Id att) const {
		return GetAttachmentSize(_attachment, att);
	}
	const char* GLFBuffer::getResourceName() const noexcept {
		return "GLFBuffer";
	}
	HTex GLFBuffer::getAttachmentAsTexture(Att::Id id) const {
		return boost::apply_visitor(GetTex_Visitor(), _attachment[id]);
	}
	HRb GLFBuffer::getAttachmentAsRBuffer(Att::Id id) const {
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
		(*tbl)["Color0"] = lua_Integer(Att::COLOR0);
		#ifndef USE_OPENGLES2
			(*tbl)["Color1"] = lua_Integer(Att::COLOR1);
			(*tbl)["Color2"] = lua_Integer(Att::COLOR2);
			(*tbl)["Color3"] = lua_Integer(Att::COLOR3);
		#endif
		(*tbl)["Depth"] = lua_Integer(Att::DEPTH);
		(*tbl)["Stencil"] = lua_Integer(Att::STENCIL);
		(*tbl)["DepthStencil"] = lua_Integer(Att::DEPTH_STENCIL);
		lsc.setField(-1, "Attribute", tbl);
	}
}
