#include "uniform_ent.hpp"
#include "gl_program.hpp"

namespace rev {
	bool UniformEnt::_refresh(IdEntry::value_t& dst, IdEntry*) const {
		dst.clear();
		return true;
	}
	void UniformEnt::assign(const UniformEnt& e) {
		setProgram(e.getProgram());
		setIdEntry(e.getIdEntry());
	}
	void UniformEnt::clearValue() {
		refIdEntry().clear();
	}
	void UniformEnt::_setUniform(const GLint id, const draw::Cmd_SP& cmd) {
		getIdEntry();
		auto& e = refIdEntry();
		auto itr = e.find(id);
		if(itr != e.end()) {
			itr->second = std::move(cmd);
		} else
			e.emplace(id, std::move(cmd));
	}
	void UniformEnt::dcmd_export(draw::IQueue& q) {
		const auto& prog = getProgram();
		const auto& ent = getIdEntry();
		for(auto& e : ent) {
			const auto id = e.first;
			// テクスチャユニット番号を検索
			const auto num = prog->getTexIndex(id);
			e.second->dcmd_export(q, id, (num ? *num : -1));
		}
	}
}
