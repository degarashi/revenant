#pragma once
#include "spine/rflag.hpp"
#include "drawtoken/tokenml.hpp"
#include "handle/opengl.hpp"
#include "gl_program.hpp"
#include "drawtoken/make_uniform.hpp"

namespace rev {
	namespace draw {
		struct Token;
		using Token_SP = std::shared_ptr<Token>;
	}
	class UniformEnt :
		public IDebugGui
	{
		private:
			using UniformIdMap_t = std::unordered_map<int, draw::Token_SP>;
			struct Result_t {
				draw::TokenML				token;
			};
			#define SEQ \
				((Program)(HProg)) \
				((IdEntry)(UniformIdMap_t)(Program)) \
				((Result)(Result_t)(Program)(IdEntry))
			RFLAG_DEFINE(UniformEnt, SEQ)
			RFLAG_SETMETHOD(IdEntry)
			RFLAG_REFMETHOD(IdEntry)
		public:
			RFLAG_GETMETHOD_DEFINE(SEQ)
			RFLAG_SETMETHOD_DEFINE(SEQ)
			RFLAG_REFMETHOD_DEFINE(SEQ)
			#undef SEQ

			template <class T, class Make, ENABLE_IF(!std::is_integral_v<T>)>
			bool setUniformWithMake(const T& name, Make&& make) {
				if(const auto id = getProgram()->getUniformId(name)) {
					setUniformToken(*id, make());
					return true;
				}
				return false;
			}
			template <class T, class... Ts, ENABLE_IF(!std::is_integral_v<T>)>
			bool setUniform(const T& name, Ts&&... ts) {
				return setUniformWithMake(name, [&ts...](){
					return draw::MakeUniform(std::forward<Ts>(ts)...);
				});
			}
			void setUniformToken(const int id, const draw::Token_SP& t);
			template <class... Ts>
			void setUniform(const int id, Ts&&... ts) {
				setUniformToken(id, draw::MakeUniform(std::forward<Ts>(ts)...));
			}

			void copyFrom(const UniformEnt& e);
			void clearValue();

			DEF_DEBUGGUI_NAME
			DEF_DEBUGGUI_PROP
	};
}
