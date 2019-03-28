#include "shader.hpp"
#include "if.hpp"
#include "error.hpp"
#include "../msg/handler.hpp"

namespace rev {
	// ---------------------- GLShader ----------------------
	void GLShader::_initShader() {
		_idSh = GL.glCreateShader(c_glShFlag[_flag]);
		D_Assert0(_idSh != 0);

		const auto* pStr = _source.c_str();
		GL.glShaderSource(_idSh, 1, &pStr, nullptr);
		GL.glCompileShader(_idSh);

		// エラーが無かったか確認
		GLint compiled;
		GL.glGetShaderiv(_idSh, GL_COMPILE_STATUS, &compiled);
		if(compiled != GL_TRUE)
			throw GLE_ShaderError(_source, _idSh);
	}
	GLShader::GLShader() {}
	GLShader::GLShader(const ShType flag, const std::string& src):
		_idSh(0),
		_flag(flag),
		_source(src)
	{
		_initShader();
	}
	GLShader::~GLShader() {
		onDeviceLost();
	}
	bool GLShader::isEmpty() const {
		return _source.empty();
	}
	int GLShader::getShaderId() const {
		return _idSh;
	}
	void GLShader::onDeviceLost() {
		if(_idSh!=0) {
			GLW.getDrawHandler().postExecNoWait([buffId=getShaderId()](){
				D_GLWarn(glDeleteShader, buffId);
			});
			_idSh = 0;
		}
	}
	void GLShader::onDeviceReset() {
		if(!isEmpty() && _idSh==0)
			_initShader();
	}
	ShType GLShader::getShaderType() const {
		return _flag;
	}
}
