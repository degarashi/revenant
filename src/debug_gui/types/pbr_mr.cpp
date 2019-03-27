#include "../../pbr_mr.hpp"
#include "../print.hpp"
#include "../entry_field.hpp"
#include "../popup.hpp"
#include "../resource_window.hpp"
#include "../../gl/texture.hpp"
#include "../../gl/resource.hpp"

namespace rev {
	const char* PBRMetallicRoughness::getDebugName() const noexcept {
		return "PBRMetallicRoughness";
	}
	bool PBRMetallicRoughness::property(const bool edit) {
		auto f = debug::EntryField(nullptr, edit, 2);
		const auto showTex = [&f](auto& t, const char* entTex, const char* entId){
			f.entry(entId, t.coordId);
			debug::Show(nullptr, entTex);
			ImGui::NextColumn();
			if(t.tex) {
				t.tex->summary();
				if(ImGui::IsItemClicked(1)) {
					ImGui::OpenPopup(entTex);
				}
				if(const auto _ = debug::PopupPush(entTex)) {
					if(ImGui::Selectable("texture property..."))
						debug::ResourceWindow::Add(t.tex);
				}
			} else {
				debug::Show(nullptr, "(none)");
			}
			ImGui::NextColumn();
		};
		showTex(color, "ColorTexture", "ColorCoordId");
		f.entry("ColorFactor", colorFactor);

		showTex(mr, "MRTexture", "MRCoordId");
		f.entry("MR-Factor", mrFactor);

		showTex(normal, "NormalTexture", "NormalCoordId");
		f.entry("NormalScale", normalScale);

		showTex(occlusion, "OcclusionTexture", "OcclusionCoordId");
		f.entry("OcclusionStrength", occlusionStrength);

		showTex(emissive, "EmissiveTexture", "EmissiveCoordId");
		f.entry("EmissiveFactor", emissiveFactor);

		f.entry("AlphaMode", alphaMode);
		f.entry("AlphaCutoff", alphaCutoff);
		f.entry("DoubleSided", doubleSided);
		return f.modified();
	}
}
