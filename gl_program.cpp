#include "gl_program.hpp"
#include "gl_shader.hpp"
#include "gl_error.hpp"
#include "gl_resource.hpp"
#include "handler.hpp"
#include "lubee/meta/countof.hpp"
#include "drawtoken/program.hpp"

namespace rev {
	// ------------------ GLParamInfo ------------------
	GLParamInfo::GLParamInfo(const GLSLFormatDesc& desc):
		GLSLFormatDesc(desc)
	{}
	// ---------------------- GLProgram ----------------------
	void GLProgram::_setShader(const HSh& hSh) {
		if(hSh)
			_shader[hSh->getShaderType()] = hSh;
	}
	void GLProgram::_initProgram() {
		_idProg = GL.glCreateProgram();
		for(int i=0 ; i<static_cast<int>(ShType::_Num) ; i++) {
			auto& sh = _shader[i];
			// Geometryシェーダー以外は必須
			if(sh) {
				GL.glAttachShader(_idProg, sh->getShaderId());
				D_GLAssert0();
			} else {
				if(i != ShType::Geometry)
					throw GLE_Error("missing shader elements (vertex or fragment)");
			}
		}
		GL.glLinkProgram(_idProg);
		// エラーが無いかチェック
		int ib;
		GL.glGetProgramiv(_idProg, GL_LINK_STATUS, &ib);
		if(ib != GL_TRUE)
			throw GLE_ProgramError(_idProg);
		_makeTexIndex();
	}
	GLProgram::~GLProgram() {
		if(mgr_gl.isInDtor()) {
			for(auto& p : _shader)
				p.reset();
		}
		onDeviceLost();
	}
	void GLProgram::onDeviceLost() {
		if(_idProg != 0) {
			GLW.getDrawHandler().postExecNoWait([buffId=getProgramId()](){
				GLuint num;
				D_GLWarn(glGetIntegerv, GL_CURRENT_PROGRAM, reinterpret_cast<GLint*>(&num));
				if(num == buffId)
					D_GLWarn(glUseProgram, 0);
				// ShaderはProgramをDeleteすれば自動的にdetachされる
				D_GLWarn(glDeleteProgram, buffId);
			});
			_idProg = 0;
			_texIndex.clear();
		}
	}
	void GLProgram::onDeviceReset() {
		if(_idProg == 0) {
			// 先にshaderがresetされてないかもしれないので、ここでしておく
			for(auto& s : _shader) {
				if(s)
					s->onDeviceReset();
			}
			_initProgram();
		}
	}
	const HSh& GLProgram::getShader(const ShType type) const {
		return _shader[type];
	}
	GLint_OP GLProgram::getUniformId(const std::string& name) const {
		GLint id = GL.glGetUniformLocation_NC(getProgramId(), name.c_str());
		return id>=0 ? GLint_OP(id) : GLint_OP(spi::none);
	}
	GLint_OP GLProgram::getAttribId(const std::string& name) const {
		GLint id = GL.glGetAttribLocation_NC(getProgramId(), name.c_str());
		return id>=0 ? GLint_OP(id) : GLint_OP(spi::none);
	}
	GLuint GLProgram::getProgramId() const {
		return _idProg;
	}
	int GLProgram::_getNumParam(GLenum flag) const {
		int iv;
		GL.glGetProgramiv(getProgramId(), flag, &iv);
		return iv;
	}
	int GLProgram::getNActiveAttribute() const {
		return _getNumParam(GL_ACTIVE_ATTRIBUTES);
	}
	int GLProgram::getNActiveUniform() const {
		return _getNumParam(GL_ACTIVE_UNIFORMS);
	}
	namespace {
		// GLSL変数名の最大がよくわからない (ので、数は適当)
		constexpr int MaxGLSLName = 0x100;
	}
	GLParamInfo GLProgram::_getActiveParam(const int n, const InfoF infoF) const {
		GLchar buff[MaxGLSLName];
		GLsizei len;
		GLint sz;
		GLenum typ;
		(GL.*infoF)(getProgramId(), n, countof(buff), &len, &sz, &typ, buff);
		GLParamInfo ret = *GLFormat::QueryGLSLInfo(typ);
		ret.name = buff;
		return ret;
	}
	GLParamInfo GLProgram::getActiveAttribute(const int n) const {
		return _getActiveParam(n, &IGL::glGetActiveAttrib);
	}
	GLParamInfo GLProgram::getActiveUniform(const int n) const {
		return _getActiveParam(n, &IGL::glGetActiveUniform);
	}
	void GLProgram::use() const {
		GL.glUseProgram(getProgramId());
	}
	void GLProgram::getDrawToken(draw::TokenDst& dst) const {
		using UT = draw::Program;
		new(dst.allocate_memory( sizeof(UT), draw::CalcTokenOffset<UT>()))
			UT(const_cast<GLProgram*>(this)->shared_from_this());
	}
	void GLProgram::_makeTexIndex() {
		const GLint nUnif = getNActiveUniform();
		// Sampler2D変数が見つかった順にテクスチャIdを割り振る
		GLint curI = 0;
		for(GLint i=0 ; i<nUnif ; i++) {
			const auto info = getActiveUniform(i);
			if(info.type == GLSLType::TextureT) {
				// GetActiveUniformでのインデックスとGetUniformLocationIdは異なる場合があるので・・
				const GLint id = *getUniformId(info.name);
				_texIndex.emplace(id, curI++);
			}
		}
		D_GLAssert0();
	}
	const GLProgram::TexIndex& GLProgram::getTexIndex() const noexcept {
		return _texIndex;
	}
}
