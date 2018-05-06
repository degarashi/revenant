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
				UniformSetF_V tmp;
				this->T0::extractUniform(tmp, prog);
				dst.emplace_back([tmp = std::move(tmp)](const void* self, UniformEnt& u){
					for(auto& e : tmp)
						e(static_cast<const T0*>(self), u);
				});
				_extractUniform(dst, prog, ts...);
			}
			void _moveFrom(ISystemUniform&) {}
			template <class T0, class... Ts>
			void _moveFrom(ISystemUniform& prev, T0*, const Ts*... ts) {
				this->T0::moveFrom(prev);
				_moveFrom(prev, ts...);
			}
			void _applyUniform(const UniformSetF*, UniformEnt&) const {}
			template <class T0, class... Ts>
			void _applyUniform(const UniformSetF* unif, UniformEnt& u, T0*, const Ts*... ts) const {
				(*unif)(static_cast<const T0*>(this), u);
				_applyUniform(unif+1, u, ts...);
			}

		public:
			void applyUniform(UniformEnt& u, const GLProgram& prog) const override {
				auto& unif = prog.extractSystemUniform(*this);
				_applyUniform(unif.data(), u, (Base*)nullptr...);
			}
			void extractUniform(UniformSetF_V& dst, const GLProgram& prog) const override {
				_extractUniform(dst, prog, (Base*)nullptr...);
			}
			void moveFrom(ISystemUniform& prev) override {
				_moveFrom(prev, (Base*)nullptr...);
			}
	};
}
