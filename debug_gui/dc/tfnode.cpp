#include "../../dc/node.hpp"
#include "../../imgui/imgui.h"
#include "../sstream.hpp"
#include "../entry_field.hpp"
#include <boost/format.hpp>

namespace rev::dc {
	const char* TfNode::getDebugName() const noexcept {
		return "TfNode";
	}
	bool TfNode::property(const bool edit) {
		auto f = debug::EntryField(nullptr, edit);
		f.entry("Id", id);
		f.entry("JointName", (jointName ? jointName->c_str() : "<nullptr>"));
		f.show("Transform", spi::UnwrapAcValue(getTransform()));
		ImGui::Columns(1);
		ImGui::Spacing();
		if(edit) {
			auto tmp = getPose();
			if(debug::Edit("Pose", tmp))
				setPose(tmp);
		} else
			debug::Show("Pose", getPose());
		return f.modified();
	}
	void TfNode::summary() const {
		ImGui::TextUnformatted(summary_str().c_str());
	}
	std::string TfNode::summary_str() const {
		boost::format fmt(R"("%1%" (Id: %2%, JointName=%3%))");
		fmt = fmt % userName;
		fmt = fmt % id;
		fmt = fmt % (jointName ? jointName->c_str() : "null");
		return fmt.str();
	}
}
