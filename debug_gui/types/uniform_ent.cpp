#include "../../uniform_ent.hpp"
#include "../../imgui/imgui.h"
#include "../../debug_gui/column.hpp"
#include "../../debug_gui/indent.hpp"

namespace rev {
	const char* UniformEnt::getDebugName() const noexcept {
		return "UniformEnt";
	}
	bool UniformEnt::property(const bool) {
		if(const auto& p = getProgram()) {
			const auto _ = debug::IndentPush();
			const auto __ = debug::ColumnPush(2);
			const auto& ent = getIdEntry();
			for(auto& e : ent) {
				const auto* name = p->getUniformName(e.first);
				D_Assert0(name);
				ImGui::TextUnformatted(name);
				ImGui::NextColumn();

				// auto* u = dynamic_cast<const draw::IUniform*>(e.second.get());
				// D_Assert0(u);
				// u->printValue();
				ImGui::NextColumn();
			}
		}
		return false;
	}
}
