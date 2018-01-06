#include "material.hpp"
#include "gl_state.hpp"

namespace rev {
	bool Material::_refresh(M_Uniform::value_t& dst, M_Uniform*) const {
		dst.setProgram(getBase()->getProgram());
		return true;
	}
	bool Material::_refresh(F_DefaultValue::value_t& dst, F_DefaultValue*) const {
		dst.copyFrom(getBase()->getDefaultValue());
		auto& ent = getM_Uniform().getIdEntry();
		for(auto& e : ent)
			dst.setUniform(e.first, e.second);
		return true;
	}
	bool Material::_refresh(F_Setting::value_t& dst, F_Setting*) const {
		dst.clear();
		const GLState_SPV &base = getBase()->getSetting(),
						&mtl = getM_Setting();
		for(auto& b : base)
			dst.emplace_back(b);
		for(auto& m : mtl)
			dst.emplace_back(m);
		return true;
	}

	UniformEnt& Material::refM_Uniform() {
		getM_Uniform();
		return _rflag.ref<M_Uniform>();
	}
	const GLState_SPV& Material::getSetting() const {
		return getF_Setting();
	}
	const UniIdSet& Material::getNoDefaultValue() const noexcept {
		static const UniIdSet c_empty;
		return c_empty;
	}
	const VSemAttrV& Material::getVAttr() const noexcept {
		return getBase()->getVAttr();
	}
	const HProg& Material::getProgram() const noexcept {
		return getBase()->getProgram();
	}
	const UniformEnt& Material::getDefaultValue() const noexcept {
		return getF_DefaultValue();
	}
	const Name& Material::getName() const noexcept {
		return getBase()->getName();
	}
}
