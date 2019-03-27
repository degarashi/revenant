#include "renderbuffer.hpp"
#include "if.hpp"
#include "resource.hpp"
#include "error.hpp"
#include "framebuffer_tmp.hpp"
#include "../handler.hpp"

namespace rev {
	// ------------------------- GLRBuffer -------------------------
	GLRBuffer::GLRBuffer(int w, int h, GLInRenderFmt fmt):
		_idRbo(0), _behLost(NONE), _restoreInfo(boost::blank()), _buffFmt(GLFormat::QueryInfo(fmt)->elementType), _fmt(fmt), _size(w,h)
	{}
	GLRBuffer::~GLRBuffer() {
		onDeviceLost();
	}
	void GLRBuffer::onDeviceReset() {
		if(_idRbo == 0) {
			GL.glGenRenderbuffers(1, &_idRbo);
			use_begin();
			allocate();
			cs_onReset[_behLost](mgr_gl.getTmpFramebuffer(), *this);
		}
	}
	void GLRBuffer::onDeviceLost() {
		if(_idRbo != 0) {
			// 復帰処理が必要な場合はここでする
			cs_onLost[_behLost](mgr_gl.getTmpFramebuffer(), *this);

			GLW.getDrawHandler().postExecNoWait([buffId=_idRbo](){
				GLuint num;
				D_GLWarn(glGetIntegerv, GL_RENDERBUFFER_BINDING, reinterpret_cast<GLint*>(&num));
				if(num == buffId)
					D_GLWarn(glBindRenderbuffer, GL_RENDERBUFFER, 0);
				D_GLWarn(glDeleteRenderbuffers, 1, &buffId);
			});

			_idRbo = 0;
		}
	}
	void GLRBuffer::allocate() {
		GL.glRenderbufferStorage(GL_RENDERBUFFER, _fmt.get(), _size.width, _size.height);
	}

	namespace {
		void Nothing(GLFBufferTmp&, GLRBuffer&) {}
	}
	const GLRBuffer::F_LOST GLRBuffer::cs_onLost[] = {
		Nothing,		// NONE
		Nothing,		// CLEAR
		#ifndef USE_OPENGLES2
			[](GLFBufferTmp& fb, GLRBuffer& rb) {		// RESTORE
				fb.use_begin();
				fb.attachRBuffer(GLFBufferTmp::Att::Color0, rb._idRbo);
				GLFormat::Info_OP op = GLFormat::QueryInfo(rb._fmt.get());
				int texSize;
				if(op) {
					rb._buffFmt = op->elementType;
					texSize = op->numElem * (*GLFormat::QuerySize(op->numElem));
				} else {
					// BaseFormatな時は判別をかける
					uint32_t dsc = GLFormat::QueryFormat(rb._fmt.get(), GLFormat::Query_DSC);
					switch(dsc) {
						case GLFormat::Internal:		// GL_RGBA8
							rb._buffFmt = GL_UNSIGNED_BYTE;
							texSize = sizeof(GLubyte)*4;
							break;
						case GLFormat::Depth:			// GL_DEPTH_COMPONENT16
							rb._buffFmt = GL_UNSIGNED_SHORT;
							texSize = sizeof(GLushort);
							break;
					#ifndef USE_OPENGLES2
						case GLFormat::DepthStencil:	// GL_DEPTH24_STENCIL8
							rb._buffFmt = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
							texSize = sizeof(GLuint);
							break;
					#endif
						case GLFormat::Stencil:			// GL_STENCIL_INDEX8
							rb._buffFmt = GL_UNSIGNED_BYTE;
							texSize = sizeof(GLubyte);
							break;
					}
				}
				ByteBuff buff(texSize * rb._size.width * rb._size.height);
				GL.glReadPixels(0, 0, rb._size.width, rb._size.height, rb._fmt.get(), rb._buffFmt.get(), &buff[0]);
				rb._restoreInfo = std::move(buff);
			}
		#else
			// OpenGL ES2ではDepth, StencilフォーマットのRenderBuffer読み取りは出来ない
			// Colorフォーマットだったら一応可能だがRenderBufferに書き戻すのが手間なのであとで対応
			Nothing
		#endif
	};

	const GLRBuffer::F_LOST GLRBuffer::cs_onReset[] = {
		Nothing,								// NONE
		[](GLFBufferTmp& fb, GLRBuffer& rb) {		// CLEAR
			const frea::Vec4& c = boost::get<frea::Vec4>(rb._restoreInfo);
			GL.glClearColor(c.x, c.y, c.z, c.w);
			fb.use_begin();
			fb.attachRBuffer(GLFBuffer::Att::Color0, rb._idRbo);
			GL.glClear(GL_COLOR_BUFFER_BIT);
		},
		// OpenGL ES2ではglDrawPixelsが使えないので、ひとまず無効化
		#ifndef USE_OPENGLES2
			[](GLFBufferTmp& fb, GLRBuffer& rb) {		// RESTORE
				auto& buff = boost::get<ByteBuff>(rb._restoreInfo);
				fb.use_begin();
				fb.attachRBuffer(GLFBuffer::Att::Color0, rb._idRbo);
				GL.glDrawPixels(0,0, rb._fmt.get(), rb._buffFmt, &buff[0]);
				rb._restoreInfo = boost::blank();
			}
		#else
			Nothing
		#endif
	};

	void GLRBuffer::use_begin() const {
		GL.glBindRenderbuffer(GL_RENDERBUFFER, _idRbo);
		GLAssert0();
	}
	GLuint GLRBuffer::getBufferId() const {
		return _idRbo;
	}
	void GLRBuffer::setOnLost(OnLost beh, const frea::Vec4* color) {
		_behLost = beh;
		if(beh == CLEAR)
			_restoreInfo = *color;
	}
	const lubee::SizeI& GLRBuffer::getSize() const {
		return _size;
	}
	const char* GLRBuffer::getResourceName() const noexcept {
		return "GLRBuffer";
	}
}
