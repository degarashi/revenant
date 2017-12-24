#pragma once
#include "gl_format.hpp"
#include "gl_types.hpp"
#include "gl_if.hpp"
#include "drawtoken/token.hpp"

namespace rev {
	struct GLParamInfo : GLSLFormatDesc {
		std::string name;
		GLParamInfo() = default;
		GLParamInfo(const GLParamInfo&) = default;
		GLParamInfo(const GLSLFormatDesc& desc);
	};
	//! GLSLプログラムクラス
	class GLProgram : public IGLResource, public std::enable_shared_from_this<GLProgram> {
		private:
			HSh			_shader[ShType::_Num];
			GLuint		_idProg;

			void _initProgram();
			using InfoF = void (IGL::*)(GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*);
			GLParamInfo _getActiveParam(int n, InfoF infoF) const;
			int _getNumParam(GLenum flag) const;
			void _setShader(const HSh& hSh);

			void _init() {
				_initProgram();
			}
			template <class T, class... Ts>
			void _init(const T& t, const Ts&... ts) {
				_setShader(t);
				_init(ts...);
			}

		public:
			template <class... Ts>
			GLProgram(const Ts&... ts) {
				_init(ts...);
			}
			~GLProgram() override;
			void onDeviceLost() override;
			void onDeviceReset() override;
			void getDrawToken(draw::TokenDst& dst) const;
			const HSh& getShader(ShType type) const;
			GLint_OP getUniformId(const std::string& name) const;
			GLint_OP getAttribId(const std::string& name) const;
			GLuint getProgramId() const;
			int getNActiveAttribute() const;
			int getNActiveUniform() const;
			GLParamInfo getActiveAttribute(int n) const;
			GLParamInfo getActiveUniform(int n) const;
			void use() const;
	};
}
