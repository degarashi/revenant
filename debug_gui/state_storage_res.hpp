#pragma once
#include <unordered_map>
#include <memory>

using ImGuiID = unsigned int;
namespace rev {
	struct Resource;
	namespace debug {
		class StateStorage_Res {
			private:
				using Value = std::weak_ptr<Resource>;
				using Map = std::unordered_map<ImGuiID, Value>;
				static Map s_map;
			public:
				// キーに対応する値が無ければnullptrを返却
				template <class T>
				static std::shared_ptr<T> Get(const ImGuiID id) {
					return std::dynamic_pointer_cast<T>(s_map[id].lock());
				}
				static void Set(const ImGuiID id, const std::shared_ptr<Resource>& p) {
					s_map[id] = p;
				}
		};
	}
}
