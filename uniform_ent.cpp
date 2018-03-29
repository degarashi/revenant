#include "uniform_ent.hpp"
#include "gl_program.hpp"
#include "drawtoken/texture.hpp"
#include "drawtoken/make_uniform.hpp"

namespace rev {
	bool UniformEnt::_refresh(IdEntry::value_t& dst, IdEntry*) const {
		dst.clear();
		return true;
	}
	bool UniformEnt::_refresh(Result::value_t& dst, Result*) const {
		dst.token.clear();
		const auto& prog = getProgram();
		const auto& ent = getIdEntry();
		for(auto& e : ent) {
			const auto id = e.first;
			// テクスチャユニット番号を検索
			const auto num = prog->getTexIndex(id);
			e.second->exportToken(dst.token, id, (num ? *num : -1));
		}
		return true;
	}
	void UniformEnt::copyFrom(const UniformEnt& e) {
		setProgram(e.getProgram());
		setIdEntry(e.getIdEntry());
	}
	void UniformEnt::clearValue() {
		refIdEntry().clear();
	}
	void UniformEnt::setUniform(const int id, const draw::Token_SP& t) {
		getIdEntry();
		refIdEntry()[id] = t;
	}
}
