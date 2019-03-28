#include "../../info/gpu.hpp"
#include "../entry_field.hpp"
#include "../textfilter.hpp"
#include "../child.hpp"
#include "../spacing.hpp"
#include <boost/format.hpp>

namespace rev::info {
	const char* GPU::getDebugName() const noexcept {
		return "GPUInfo";
	}
	bool GPU::property(const bool edit) {
		auto field = debug::EntryField("GPUInfo", edit);
		const auto showVersion = [&field](const char* name, const auto& ver){
			const auto fmt = boost::format("%1%.%2%.%3%") % ver.major % ver.minor % ver.revision;
			field.show(name, fmt.str());
		};
		showVersion("OpenGL Version", version());
		showVersion("GLSL Version", glslVersion());
		showVersion("Driver Version", driverVersion());
		field.show("Vendor", vendor());
		field.show("Renderer", renderer());

		ImGui::Columns(1);
		ImGui::Spacing();
		field.show("Caps");
		ImGui::Separator();

		debug::TextFilter filter(ImGui::GetID("filter"));
		if(const auto caps = debug::ChildPush("caps", 0)) {
			for(auto& c : refCapabilitySet()) {
				if(filter.PassFilter(c.c_str()))
					ImGui::TextUnformatted(c.c_str());
			}
		}
		return field.modified();
	}
}
