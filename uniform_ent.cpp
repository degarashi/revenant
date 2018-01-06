#include "uniform_ent.hpp"
#include "gl_program.hpp"
#include "drawtoken/texture.hpp"
#include "drawtoken/make_uniform.hpp"

namespace rev {
	bool UniformEnt::_refresh(Result::value_t& dst, Result*) const {
		dst.idset.clear();
		dst.token.clear();
		const auto& prog = getProgram();
		const auto& tIdx = prog->getTexIndex();
		const auto& ent = getIdEntry();
		for(auto& e : ent) {
			const auto id = e.first;
			dst.idset.emplace(id);
			// テクスチャユニット番号を検索
			const auto itr = tIdx.find(id);
			e.second->exportToken(dst.token, id, (itr != tIdx.end()) ? itr->second : -1);
		}
		return true;
	}
	void UniformEnt::copyFrom(const UniformEnt& e) {
		setIdEntry(e.getIdEntry());
	}
	void UniformEnt::clearValue() {
		refIdEntry().clear();
	}
	void UniformEnt::setUniform(const int id, const draw::Token_SP& t) {
		refIdEntry()[id] = t;
	}
}
