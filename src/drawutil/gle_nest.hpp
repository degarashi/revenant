#pragma once
#include "../effect/effect.hpp"
#include "../effect/uniform_setter.hpp"

namespace rev::drawutil {
	template <class... Base>
	class GLE_Nest :
		public Base...
	{
		private:
			using this_t = GLE_Nest<Base...>;
			void _getUniformF(UniformSetF_V&, const GLProgram&) const {}
			template <class T0, class... Ts>
			void _getUniformF(UniformSetF_V& dst, const GLProgram& prog, T0*, const Ts*... ts) const {
				if(auto f = this->T0::getUniformF(prog)) {
					dst.emplace_back([f = std::move(f)](const void* p, UniformEnt& u) {
						auto* self = static_cast<const this_t*>(p);
						auto* selfL = static_cast<const T0*>(self);
						f(selfL, u);
					});
				}
				_getUniformF(dst, prog, ts...);
			}
			void _applyUniform(const UniformSetF*, UniformEnt&) const {}
			template <class T0, class... Ts>
			void _applyUniform(const UniformSetF* unif, UniformEnt& u, T0*, const Ts*... ts) const {
				(*unif)(static_cast<const T0*>(this), u);
				_applyUniform(unif+1, u, ts...);
			}

		public:
			void applyUniform(UniformEnt& u, const GLProgram& prog) const override {
				auto& f = prog.getUniformF(*this);
				f(this, u);
			}
			UniformSetF getUniformF(const GLProgram& prog) const override {
				UniformSetF_V fv;
				_getUniformF(fv, prog, (Base*)nullptr...);
				if(!fv.empty()) {
					return [fv = std::move(fv)](const void* p, UniformEnt& u){
						for(auto& f : fv)
							f(p, u);
					};
				}
				return [](const void*, UniformEnt&) {};
			}
	};
}
