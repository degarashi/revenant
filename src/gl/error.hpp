#pragma once
#include "lubee/src/error_chk.hpp"
#include "header.hpp"
#include <functional>

// OpenGLに関するアサート集
#define GLEC_Base(flag, act, ...)		EChk_polling##flag<::lubee::err::act>(::rev::GLError(), SOURCEPOS, __VA_ARGS__)
#define GLEC_Base0(flag, act)			EChk_polling##flag<::lubee::err::act>(::rev::GLError(), SOURCEPOS)

#define GLAssert(func, ...)					GLEC_Base(_a, Trap, [&](){return GL.func(__VA_ARGS__);})
#define GLAssert0()							GLEC_Base0(_a, Trap)
#define D_GLAssert(func, ...)				GLEC_Base(_d, Trap, [&](){return GL.func(__VA_ARGS__);})
#define D_GLAssert0()						GLEC_Base0(_d, Trap)
#define GLWarn(func, ...)					GLEC_Base(_a, Warn, [&](){return GL.func(__VA_ARGS__);})
#define GLWarn0()							GLEC_Base0(_a, Warn)
#define D_GLWarn(func, ...)					GLEC_Base(_d, Warn, [&](){return GL.func(__VA_ARGS__);})
#define D_GLWarn0()							GLEC_Base0(_d, Warn)

namespace rev {
	//! OpenGLエラーIDとその詳細メッセージ
	struct GLError {
		std::string _errMsg;
		const static std::pair<GLenum, const char*> ErrorList[];

		const char* errorDesc();
		void reset() const;
		const char* getAPIName() const;
		//! エラー値を出力しなくなるまでループする
		void resetError() const;
	};
	struct GLProgError {
		GLuint	_id;
		GLProgError(GLuint id);
		const char* errorDesc() const;
		void reset() const;
		const char* getAPIName() const;
	};
	struct GLShError {
		GLuint	_id;
		GLShError(GLuint id);
		const char* errorDesc() const;
		void reset() const;
		const char* getAPIName() const;
	};
	//! OpenGLに関する全般的なエラー
	struct GLE_Error : std::runtime_error {
		static const char* GetErrorName();
		using runtime_error::runtime_error;
	};

	// ------------------ GL例外クラス ------------------
	using GLGetIV = std::function<void (GLuint, GLenum, GLint*)>;
	using GLInfoFunc = std::function<void (GLuint, GLsizei, GLsizei*, GLchar*)>;
	//! GLSLコンパイル関連のエラー基底
	struct GLE_ShProgBase : GLE_Error {
		GLGetIV 	_ivF;
		GLInfoFunc	_infoF;
		GLuint		_id;

		static const char* GetErrorName();
		GLE_ShProgBase(const GLGetIV& ivF, const GLInfoFunc& infoF, const std::string& aux, GLuint id);
	};
	//! GLSLシェーダーコンパイルエラー
	struct GLE_ShaderError : GLE_ShProgBase {
		static const char* GetErrorName();
		GLE_ShaderError(const std::string& src, GLuint id);
	};
	//! GLSLプログラムリンクエラー
	struct GLE_ProgramError : GLE_ShProgBase {
		static const char* GetErrorName();
		GLE_ProgramError(GLuint id);
	};
	//! GLSL変数が見つからないエラー
	struct GLE_ParamNotFound : GLE_Error {
		static const char* GetErrorName();
		std::string	_name;
		GLE_ParamNotFound(const std::string& name);
	};
	//! GLSLユーザー変数の型エラー
	struct GLE_InvalidArgument : GLE_Error {
		static const char* GetErrorName();
		std::string	_shName, _argName;
		GLE_InvalidArgument(const std::string& shname, const std::string& argname);
	};
	//! GLXファイルの論理的な記述ミス
	struct GLE_LogicalError : GLE_Error {
		static const char* GetErrorName();
		using GLE_Error::GLE_Error;
	};
}
