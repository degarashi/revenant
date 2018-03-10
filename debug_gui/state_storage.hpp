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
		template <class V>
		class StateStorage {
			private:
				using Value = std::shared_ptr<V>;
				using Map = std::unordered_map<ImGuiID, Value>;
				static Map s_map;

			public:
				// キーに対応する値が無ければnullptrを返却
				static Value Get(const ImGuiID id) {
					auto itr = s_map.find(id);
					if(itr == s_map.end())
						return nullptr;
					return itr->second;
				}
				// キーに対応する値が無ければコールバックで作成
				template <class Make>
				static const Value& Get(const ImGuiID id, Make&& make) {
					auto itr = s_map.find(id);
					if(itr == s_map.end())
						itr = s_map.emplace(id, make()).first;
					return itr->second;
				}
				// キーに対応する値が無ければデフォルト初期化
				static const Value& GetDefault(const ImGuiID id) {
					return Get(id, [](){ return std::make_shared<V>(); });
				}
		};
		template <class V>
		typename StateStorage<V>::Map StateStorage<V>::s_map;
	}
}
