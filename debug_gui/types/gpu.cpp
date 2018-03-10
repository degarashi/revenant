#include "../../gpu.hpp"
#include "../entry_field.hpp"
#include "../textfilter.hpp"
#include <boost/format.hpp>

namespace rev {
	void GPUInfo::guiViewer(bool) {
		if(auto field = debug::EntryField("GPUInfo")) {
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
			if(const auto caps = debug::ChildPush("caps", {0,0})) {
				for(auto& c : refCapabilitySet()) {
					if(filter.PassFilter(c.c_str()))
						ImGui::TextUnformatted(c.c_str());
				}
			}
		}
	}
}
