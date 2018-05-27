#pragma once
#include "gl_format.hpp"
#include "gl_types.hpp"
#include "gl_if.hpp"
#include "handle/opengl.hpp"
#include "spine/flyweight_item.hpp"
#include <typeindex>

namespace rev {
	namespace draw {
		class IQueue;
	}
	using Name = std::string;
	using SName = spi::FlyweightItem<std::string>;
	class UniformEnt;
	using UniformSetF = std::function<void (const void*, UniformEnt&)>;
	using UniformSetF_V = std::vector<UniformSetF>;

	//! GLSLプログラムクラス
	class GLProgram :
		public IGLResource,
		public std::enable_shared_from_this<GLProgram>
	{
		private:
			struct GLParamInfo : GLSLFormatDesc {
				SName	name;
				GLuint	id;
				GLsizei	length;
				bool	bInteger;

				GLParamInfo() = default;
				GLParamInfo(const GLParamInfo&) = default;
				GLParamInfo(const GLSLFormatDesc& desc);
				#ifdef DEBUGGUI_ENABLED
					void showAsRow() const;
				#endif
			};
			using UniformMap = std::unordered_map<SName, GLParamInfo>;
			using AttribMap = UniformMap;
			using TexIndex = std::unordered_map<GLint, GLint>;
			struct DCmd_Use {
				GLuint		progId;
				static void Command(const void* p);
			};

			HSh					_shader[ShType::_Num];
			GLuint				_idProg;
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

			// SystemUniformに対応する変数のリスト
			// [typeid(SystemUniform)] -> UniformSetF
			using SysUSetF_M = std::unordered_map<std::type_index, UniformSetF>;
			mutable SysUSetF_M		_sysUniform;

		public:
			template <class... Shader>
			GLProgram(const Shader&... shader) {
				_initShader(shader...);
				_initProgram();
			}
			~GLProgram() override;
			void onDeviceLost() override;
			void onDeviceReset() override;
			void dcmd_export(draw::IQueue& q) const;
			const HSh& getShader(ShType type) const noexcept;

			//! 文字列によるUniform-Id検索
			GLint_OP getUniformId(const SName& name) const;
			//! 文字列によるAttribute検索
			spi::Optional<const GLParamInfo&> getAttrib(const SName& name) const;

			GLuint getProgramId() const noexcept;
			const UniformMap& getUniform() const noexcept;
			const AttribMap& getAttrib() const noexcept;
			void use() const;
			GLint_OP getTexIndex(GLint id) const;

			template <class S>
			const UniformSetF& getUniformF(const S& s) const {
				const auto idx = std::type_index(typeid(s));
				const auto itr = _sysUniform.find(idx);
				if(itr != _sysUniform.end())
					return itr->second;
				return _sysUniform.emplace(idx, s.getUniformF(*this)).first->second;
			}

			// デバッグ用(線形探索なので遅い)
			const char* getUniformName(GLint id) const;
			DEF_DEBUGGUI_NAME
			DEF_DEBUGGUI_PROP
	};
}
