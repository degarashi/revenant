#pragma once
#include <memory>
#include <unordered_set>

namespace rev {
	struct Resource;
	namespace debug {
		class ResourceWindow {
			private:
				using SP = std::shared_ptr<Resource>;
				using WP = std::weak_ptr<Resource>;
				struct Entry {
					const void*	ptr;
					WP			wp;

					Entry() = default;
					Entry(const void* p, WP wp);
					bool operator == (const Entry& e) const noexcept;
					std::size_t operator ()(const Entry& e) const noexcept;
				};
				using Set = std::unordered_set<Entry, Entry>;
				static Set	s_set;
			public:
				static void Add(const SP& sp);
				static void Draw();
		};
	}
}
