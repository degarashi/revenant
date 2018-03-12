#include "../../interval.hpp"
#include "../../imgui/imgui.h"
#include "../print.hpp"
#include "../spacing.hpp"

namespace rev {
	namespace interval {
		bool Wait::guiEditor(bool) {
			ImGui::TextUnformatted("Wait: ");
			ImGui::SameLine();
			return debug::Edit("Wait", wait);
		}

		bool EveryN::guiEditor(bool) {
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
		bool Accum::guiEditor(bool) {
			ImGui::TextUnformatted("Accum: ");
			ImGui::SameLine();
			debug::Show("Accum", accum);

			ImGui::TextUnformatted("Skipped: ");
			ImGui::SameLine();
			debug::Show("Skipped", skipped);
			return false;
		}
	}
}
