#include "gl_program.hpp"
#include "gl_shader.hpp"
#include "gl_error.hpp"
#include "gl_resource.hpp"
#include "handler.hpp"
#include "lubee/meta/countof.hpp"
#include "drawcmd/queue_if.hpp"

namespace rev {
	// ------------------ GLProgram::GLParamInfo ------------------
	GLProgram::GLParamInfo::GLParamInfo(const GLSLFormatDesc& desc):
		GLSLFormatDesc(desc)
	{}

	// ---------------------- GLProgram ----------------------
	namespace {
		// GLSL変数名の最大値
		constexpr int MaxGLSLName = GL_ACTIVE_UNIFORM_MAX_LENGTH;
		GLchar g_buff[MaxGLSLName];

		const std::regex re_array(R"(([\w_]+)\[0\])");
	}
	GLProgram::GLParamInfo GLProgram::_getActiveParam(const std::size_t n, const InfoF infoF) const {
		GLsizei len;
		GLint sz;
		GLenum typ;
		(GL.*infoF)(getProgramId(), n, countof(g_buff), &len, &sz, &typ, g_buff);
		GLParamInfo ret = *GLFormat::QueryGLSLInfo(typ);
		ret.length = sz;
		ret.name = SName(g_buff);

		std::smatch m;
		if(std::regex_match(*ret.name, m, re_array)) {
			ret.name = m[1].str();
		}
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
			info.id = GL.glGetAttribLocation(_idProg, info.name->c_str());
			D_Assert0(info.id >= 0);
			if(info.length > 1) {
				_amap.emplace(*info.name + "[0]", info);
			}
			_amap.emplace(info.name, std::move(info));
		}
	}
	void GLProgram::_makeUniformMap() {
		const std::size_t n = _getNumParam(GL_ACTIVE_UNIFORMS);
		for(std::size_t i=0 ; i<n ; i++) {
			GLParamInfo info = _getActiveParam(i, &IGL::glGetActiveUniform);
			info.id = GL.glGetUniformLocation(_idProg, info.name->c_str());
			D_Assert0(info.id >= 0);
			if(info.length > 1) {
				GLParamInfo info2 = info;
				info2.length = 1;
				for(decltype(info.length) i=0 ; i<info.length ; i++) {
					info2.name = info.name;
					{
						auto tmp = info2.name.ref();
						*tmp += "[";
						*tmp += std::to_string(i);
						*tmp += "]";
					}
					_umap.emplace(info2.name, info2);
					++info2.id;
				}
			}
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
	GLint_OP GLProgram::getUniformId(const SName& name) const {
		if(const auto itr = _umap.find(name);
			itr != _umap.end())
			return itr->second.id;
		return spi::none;
	}
	GLint_OP GLProgram::getAttribId(const SName& name) const {
		if(const auto itr = _amap.find(name);
			itr != _amap.end())
			return itr->second.id;
		return spi::none;
	}
	void GLProgram::use() const {
		GL.glUseProgram(getProgramId());
	}
	void GLProgram::dcmd_export(draw::IQueue& q) const {
		q.add(DCmd_Use{getProgramId()});
		q.stockResource(shared_from_this());
	}
	const GLProgram::UniformMap& GLProgram::getUniform() const noexcept {
		return _umap;
	}
	const GLProgram::AttribMap& GLProgram::getAttrib() const noexcept {
		return _amap;
	}
	const char* GLProgram::getUniformName(const GLint id) const {
		const auto itr = std::find_if(_umap.cbegin(), _umap.cend(),
			[id](const auto& p){
				return GLint(p.second.id) == id;
			}
		);
		if(itr != _umap.cend())
			return itr->first->c_str();
		return nullptr;
	}

	void GLProgram::DCmd_Use::Command(const void* p) {
		auto& self = *static_cast<const GLProgram::DCmd_Use*>(p);
		GL.glUseProgram(self.progId);
	}
}
