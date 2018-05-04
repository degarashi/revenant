#pragma once
#include "../glx.hpp"
#include "../sys_uniform_if.hpp"

namespace rev::util {
	template <class... Base>
	class GLE_Nest :
		public Base...
	{
		private:
			void _outputUniforms(UniformEnt&) const {}
			template <class T0, class... Ts>
			void _outputUniforms(UniformEnt& u, T0*, const Ts*... ts) const {
				this->T0::outputUniforms(u);
				_outputUniforms(u, ts...);
			}

			void _moveFrom(ISystemUniform&) {}
			template <class T0, class... Ts>
			void _moveFrom(ISystemUniform& prev, T0*, const Ts*... ts) {
				this->T0::moveFrom(prev);
				_moveFrom(prev, ts...);
			}

		public:
			void outputUniforms(UniformEnt& u) const override {
				_outputUniforms(u, (Base*)nullptr...);
			}
			void moveFrom(ISystemUniform& prev) override {
				_moveFrom(prev, (Base*)nullptr...);
			}
	};
}
