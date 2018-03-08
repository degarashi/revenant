#pragma once
#include <unordered_map>
#include <memory>

using ImGuiID = unsigned int;
namespace rev {
	struct Resource;
	namespace debug {
		class StateStorage {
			private:
				using Map = std::unordered_map<ImGuiID, std::weak_ptr<Resource>>;
				static Map s_map;
			public:
				template <class T>
				static std::shared_ptr<T> Get(const ImGuiID id) {
					auto itr = s_map.find(id);
					if(itr == s_map.end()) {
						s_map.emplace(id, std::weak_ptr<Resource>{});
						return nullptr;
					}
					return std::dynamic_pointer_cast<T>(itr->second.lock());
				}
				static void Set(const ImGuiID id, const std::shared_ptr<Resource>& p) {
					s_map[id] = p;
				}
		};
	}
}
