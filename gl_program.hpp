#pragma once
#include "gl_format.hpp"
#include "gl_types.hpp"
#include "gl_if.hpp"
#include "handle/opengl.hpp"
#include "drawtoken/token.hpp"

namespace rev {
	using Name = std::string;
	//! GLSLプログラムクラス
	class GLProgram :
		public IGLResource,
		public std::enable_shared_from_this<GLProgram>
	{
		private:
			struct GLParamInfo : GLSLFormatDesc {
				Name	name;
				GLuint	id;
				GLParamInfo() = default;
				GLParamInfo(const GLParamInfo&) = default;
				GLParamInfo(const GLSLFormatDesc& desc);
				#ifdef DEBUGGUI_ENABLED
					void showAsRow() const;
				#endif
			};
			using LiteralUniformMap = std::unordered_map<const char*, GLint_OP>;
			using LiteralAttribMap = LiteralUniformMap;
			using UniformMap = std::unordered_map<Name, GLParamInfo>;
			using AttribMap = UniformMap;
			using TexIndex = std::unordered_map<GLint, GLint>;

			HSh					_shader[ShType::_Num];
			GLuint				_idProg;
			//! [LiteralString -> UniformId]
			mutable LiteralUniformMap	_literalUmap;
			//! [LiteralString -> AttribId]
			mutable LiteralAttribMap	_literalAmap;
			//! [UniformName -> GLParamInfo]
			UniformMap			_umap;
			//! [AttribName -> GLParamInfo]
			AttribMap			_amap;
			//! [UniformId -> TextureActiveIndex]
			TexIndex			_texIndex;

			void _initProgram();
			using InfoF = void (IGL::*)(GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*);
			GLParamInfo _getActiveParam(std::size_t n, InfoF infoF) const;
			std::size_t _getNumParam(GLenum flag) const;
			void _setShader(const HSh& hSh);

			void _initShader() {}
			template <class Sh, class... Remain>
			void _initShader(const Sh& sh, const Remain&... remain) {
				_setShader(sh);
				_initShader(remain...);
			}
			void _makeUniformMap();
			void _makeAttribMap();
			void _makeTexIndex();

		public:
			template <class... Shader>
			GLProgram(const Shader&... shader) {
				_initShader(shader...);
				_initProgram();
			}
			~GLProgram() override;
			void onDeviceLost() override;
			void onDeviceReset() override;
			void getDrawToken(draw::TokenDst& dst) const;
			const HSh& getShader(ShType type) const noexcept;

			//! リテラル文字列によるUniform-Id検索
			template <std::size_t N>
			GLint_OP getUniformId(const char (&name)[N]) const {
				return getUniformId_Literal(name);
			}
			GLint_OP getUniformId_Literal(const char* name) const;
			//! 通常の文字列によるUniform-Id検索
			GLint_OP getUniformId(const Name& name) const;

			//! リテラル文字列によるAttribute-Id検索
			template <std::size_t N>
			GLint_OP getAttribId(const char (&name)[N]) const {
				return getAttributeId_Literal(name);
			}
			GLint_OP getAttributeId_Literal(const char* name) const;
			//! 通常の文字列によるUniform-Id検索
			GLint_OP getAttribId(const Name& name) const;

			GLuint getProgramId() const noexcept;
			const UniformMap& getUniform() const noexcept;
			const AttribMap& getAttrib() const noexcept;
			void use() const;
			GLint_OP getTexIndex(GLint id) const;
			DEF_DEBUGGUI_NAME
			DEF_DEBUGGUI_PROP
	};
}
