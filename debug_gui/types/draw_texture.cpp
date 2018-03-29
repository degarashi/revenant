#include "../../drawtoken/texture.hpp"
#include "../entry_field.hpp"
#include "../../imgui_sdl2.hpp"

namespace rev::draw {
	void Texture::printValue() const {
		if(ImGui::ImageButton(mgr_gui.storeResource(_hTex), {128,128})) {}
	}

	void TextureA::printValue() const {
		std::size_t idx = 0;
		for(auto& t : _texA) {
			const debug::IdPush _(idx++);
			t.printValue();
		}
	}
}
