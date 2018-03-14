#pragma once
#include <unordered_map>
#include <memory>

using ImGuiID = unsigned int;
namespace rev {
	namespace debug {
		namespace detail {
			template <class T>
			decltype(auto) DerefWP(const std::shared_ptr<T>& p) {
				return p;
			}
			template <class T>
			std::shared_ptr<T> DerefWP(const std::weak_ptr<T>& p) {
				return p.lock();
			}
		}
		template <class P>
		class StateStorage {
			protected:
				using value_t = typename P::element_type;
				using pointer_t = P;
				using Map = std::unordered_map<ImGuiID, pointer_t>;
				static Map s_map;

			public:
				// キーに対応する値が無ければnullptrを返却
				template <class T = value_t>
				static std::shared_ptr<T> Get(const ImGuiID id) {
					auto itr = s_map.find(id);
					if(itr == s_map.end())
						return nullptr;
					return std::dynamic_pointer_cast<T>(detail::DerefWP(itr->second));
				}
				// キーに対応する値が無ければコールバックで作成
				template <class T = value_t, class Make>
				static std::shared_ptr<T> GetWithMake(const ImGuiID id, Make&& make) {
					auto itr = s_map.find(id);
					if(itr == s_map.end())
						itr = s_map.emplace(id, make()).first;
					return std::dynamic_pointer_cast<T>(detail::DerefWP(itr->second));
				}
				// キーに対応する値が無ければデフォルト初期化
				template <class T = value_t>
				static std::shared_ptr<T> GetDefault(const ImGuiID id) {
					return GetWithMake(id, [](){ return std::make_shared<T>(); });
				}
				static void Set(const ImGuiID id, const pointer_t& p) {
					s_map[id] = p;
				}
		};
		template <class P>
		typename StateStorage<P>::Map StateStorage<P>::s_map;
	}
}
