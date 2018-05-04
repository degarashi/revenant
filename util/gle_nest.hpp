#pragma once
#include "../glx.hpp"
#include "../sys_uniform_if.hpp"

namespace rev::util {
	template <class... Base>
	class GLE_Nest :
		public Base...
	{
		private:
			void _extractUniform(UniformSetF_V&, const GLProgram&) const {}
			template <class T0, class... Ts>
			void _extractUniform(UniformSetF_V& dst, const GLProgram& prog, T0*, const Ts*... ts) const {
				this->T0::extractUniform(dst, prog);
				_extractUniform(dst, prog, ts...);
			}

			void _moveFrom(ISystemUniform&) {}
			template <class T0, class... Ts>
			void _moveFrom(ISystemUniform& prev, T0*, const Ts*... ts) {
				this->T0::moveFrom(prev);
				_moveFrom(prev, ts...);
			}

		public:
			void extractUniform(UniformSetF_V& dst, const GLProgram& prog) const override {
				_extractUniform(dst, prog, (Base*)nullptr...);
			}
			void moveFrom(ISystemUniform& prev) override {
				_moveFrom(prev, (Base*)nullptr...);
			}
	};
}
