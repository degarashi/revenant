#include "u_morph.hpp"
#include "gl_program.hpp"
#include "uniform_ent.hpp"

namespace rev {
	void U_Morph::Coeff::Ar_Union::fillZero() {
		for(auto& v : ar_v)
			v = frea::Vec4(0);
	}
	void U_Morph::Coeff::fillZero() {
		pos.fillZero();
		normal.fillZero();
		tangent.fillZero();
	}

	namespace {
		using UniformName = spi::FlyweightItem<std::string>;
		namespace unif {
			namespace coeff {
				const UniformName
						pos("u_posWeight"),
						normal("u_normalWeight"),
						tangent("u_tangentWeight");
			}
		}
	}
	UniformSetF U_Morph::getUniformF(const GLProgram& prog) const {
		UniformSetF_V fv;
		const auto set = [&fv, &prog](const auto& name, const auto& cb){
			if(const auto id = prog.getUniformId(name)) {
				fv.emplace_back([id=*id, &cb](const void* p, UniformEnt& u){
					auto* self = static_cast<const U_Morph*>(p);
					u.setUniformById(id, cb(*self));
				});
			}
		};
		set(unif::coeff::pos, [](auto& s)->decltype(auto) { return s.coeff.pos.ar_v; });
		set(unif::coeff::normal, [](auto& s)->decltype(auto) { return s.coeff.normal.ar_v; });
		set(unif::coeff::tangent, [](auto& s)->decltype(auto) { return s.coeff.tangent.ar_v; });

		if(!fv.empty()) {
			return [fv = std::move(fv)](const void* p, UniformEnt& u){
				for(auto& f : fv)
					f(p, u);
			};
		}
		return nullptr;
	}
}
