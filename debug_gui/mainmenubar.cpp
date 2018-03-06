#include "mainmenubar.hpp"
#include "../imgui/imgui.h"

namespace rev {
	namespace debug {
		MainMenuBarPush::MainMenuBarPush() {
			_valid = ImGui::BeginMainMenuBar();
		}
		MainMenuBarPush::MainMenuBarPush(MainMenuBarPush&& m):
			_valid(m._valid)
		{
			m._valid = false;
		}
		MainMenuBarPush::operator bool () const noexcept {
			return _valid;
		}
		MainMenuBarPush::~MainMenuBarPush() {
			if(_valid)
				ImGui::EndMainMenuBar();
		}
	}
}
