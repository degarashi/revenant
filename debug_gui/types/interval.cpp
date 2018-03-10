#include "../../interval.hpp"
#include "../../imgui/imgui.h"
#include "../print.hpp"

namespace rev {
	namespace interval {
		bool Wait::guiEditor() {
			ImGui::TextUnformatted("Wait: ");
			ImGui::SameLine();
			return debug::Edit("Wait", wait);
		}
		float Wait::guiHeight() const {
			return ImGui::GetItemsLineHeightWithSpacing();
		}

		bool EveryN::guiEditor() {
			ImGui::TextUnformatted("Every: ");
			ImGui::SameLine();
			{
				const auto idp = debug::IdPush("EveryN");
				int tmp = nth;
				if(ImGui::SliderInt("", &tmp, 0, 60)) {
					nth = tmp;
					return true;
				}
			}
			return false;
		}
		float EveryN::guiHeight() const {
			return ImGui::GetItemsLineHeightWithSpacing();
		}
		void Accum::guiViewer() const {
			ImGui::TextUnformatted("Accum: ");
			ImGui::SameLine();
			debug::Show("Accum", accum);

			ImGui::TextUnformatted("Skipped: ");
			ImGui::SameLine();
			debug::Show("Skipped", skipped);
		}
		float Accum::guiHeight() const {
			return ImGui::GetItemsLineHeightWithSpacing()*2;
		}
	}
}