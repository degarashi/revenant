#include "id.hpp"
#include "../imgui/imgui.h"

namespace rev {
	namespace debug {
		IdPush::IdPush(std::nullptr_t):
			_valid(false)
		{}
		IdPush::IdPush(IdPush&& idp):
			_valid(idp._valid)
		{
			idp._valid = false;
		}
		IdPush::IdPush(const int id):
			_valid(true)
		{
			ImGui::PushID(id);
		}
		IdPush::IdPush(const char* id):
			_valid(true)
		{
			ImGui::PushID(id);
		}
		IdPush::IdPush(const void* id):
			_valid(true)
		{
			ImGui::PushID(id);
		}
		IdPush::~IdPush() {
			if(_valid)
				ImGui::PopID();
		}
	}
}
