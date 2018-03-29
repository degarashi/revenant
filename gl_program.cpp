#include "gl_program.hpp"
#include "gl_shader.hpp"
#include "gl_error.hpp"
#include "gl_resource.hpp"
#include "handler.hpp"
#include "lubee/meta/countof.hpp"
#include "drawtoken/program.hpp"

namespace rev {
	// ------------------ GLProgram::GLParamInfo ------------------
	GLProgram::GLParamInfo::GLParamInfo(const GLSLFormatDesc& desc):
		GLSLFormatDesc(desc)
	{}

	// ---------------------- GLProgram ----------------------
	namespace {
		// GLSL変数名の最大がよくわからない (ので、数は適当)
		constexpr int MaxGLSLName = 0x100;
	}
	GLProgram::GLParamInfo GLProgram::_getActiveParam(const std::size_t n, const InfoF infoF) const {
		GLchar buff[MaxGLSLName];
		GLsizei len;
		GLint sz;
		GLenum typ;
		(GL.*infoF)(getProgramId(), n, countof(buff), &len, &sz, &typ, buff);
		GLParamInfo ret = *GLFormat::QueryGLSLInfo(typ);
		ret.name = buff;
		return ret;
	}
	std::size_t GLProgram::_getNumParam(const GLenum flag) const {
		int iv;
		GL.glGetProgramiv(getProgramId(), flag, &iv);
		return iv;
	}
	void GLProgram::_setShader(const HSh& hSh) {
		if(hSh)
			_shader[hSh->getShaderType()] = hSh;
	}
	void GLProgram::_makeAttribMap() {
		const std::size_t n = _getNumParam(GL_ACTIVE_ATTRIBUTES);
		for(std::size_t i=0 ; i<n ; i++) {
			GLParamInfo info = _getActiveParam(i, &IGL::glGetActiveAttrib);
			info.id = GL.glGetAttribLocation(_idProg, info.name.c_str());
			D_Assert0(info.id >= 0);
			_amap.emplace(info.name, std::move(info));
		}
	}
	void GLProgram::_makeUniformMap() {
		const std::size_t n = _getNumParam(GL_ACTIVE_UNIFORMS);
		for(std::size_t i=0 ; i<n ; i++) {
			GLParamInfo info = _getActiveParam(i, &IGL::glGetActiveUniform);
			info.id = GL.glGetUniformLocation(_idProg, info.name.c_str());
			D_Assert0(info.id >= 0);
			_umap.emplace(info.name, std::move(info));
		}
	}
	void GLProgram::_makeTexIndex() {
		GLint curI = 0;
		// Sampler2D変数が見つかった順にテクスチャIdを割り振る
		for(auto& u : _umap) {
			const auto& info = u.second;
			if(info.type == GLSLType::TextureT) {
				// GetActiveUniformでのインデックスとGetUniformLocationIdは異なる場合があるので・・
				const GLint id = *getUniformId(info.name);
				_texIndex.emplace(id, curI++);
			}
		}
		D_GLAssert0();
	}
	void GLProgram::_initProgram() {
		_idProg = GL.glCreateProgram();
		for(std::size_t i=0 ; i<ShType::_Num ; i++) {
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

		_makeUniformMap();
		_makeAttribMap();
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
			_literalUmap.clear();
			_literalAmap.clear();
			_umap.clear();
			_amap.clear();
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
	const HSh& GLProgram::getShader(const ShType type) const noexcept {
		return _shader[type];
	}
	GLuint GLProgram::getProgramId() const noexcept {
		return _idProg;
	}
	GLint_OP GLProgram::getTexIndex(const GLint id) const {
		if(const auto itr = _texIndex.find(id);
			itr != _texIndex.end())
			return itr->second;
		return spi::none;
	}
	GLint_OP GLProgram::_getUniformId_Literal(const char* name) const {
		auto itr = _literalUmap.find(name);
		if(itr == _literalUmap.end()) {
			itr = _literalUmap.emplace(name, getUniformId(name)).first;
		}
		return itr->second;
	}
	GLint_OP GLProgram::_getAttributeId_Literal(const char* name) const {
		auto itr = _literalAmap.find(name);
		if(itr == _literalAmap.end()) {
			itr = _literalAmap.emplace(name, getAttribId(name)).first;
		}
		return itr->second;
	}
	GLint_OP GLProgram::getUniformId(const Name& name) const {
		if(const auto itr = _umap.find(name);
			itr != _umap.end())
			return itr->second.id;
		return spi::none;
	}
	GLint_OP GLProgram::getAttribId(const Name& name) const {
		if(const auto itr = _amap.find(name);
			itr != _amap.end())
			return itr->second.id;
		return spi::none;
	}
	void GLProgram::use() const {
		GL.glUseProgram(getProgramId());
	}
	void GLProgram::getDrawToken(draw::TokenDst& dst) const {
		using UT = draw::Program;
		new(dst.allocate_memory( sizeof(UT), draw::CalcTokenOffset<UT>()))
			UT(const_cast<GLProgram*>(this)->shared_from_this());
	}
	const GLProgram::UniformMap& GLProgram::getUniform() const noexcept {
		return _umap;
	}
	const GLProgram::AttribMap& GLProgram::getAttrib() const noexcept {
		return _amap;
	}
}
