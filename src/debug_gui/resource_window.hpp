#pragma once
#include <memory>
#include <unordered_set>
#include "../handle/gui.hpp"

namespace rev {
	struct IDebugGui;
	namespace debug {
		class ResourceWindow {
			private:
				struct Entry {
					const void*	ptr;
					WDbg		wp;

					Entry() = default;
					Entry(const void* p, WDbg wp);
					bool operator == (const Entry& e) const noexcept;
					std::size_t operator ()(const Entry& e) const noexcept;
				};
				using Set = std::unordered_set<Entry, Entry>;
				static Set	s_set;
			public:
				static void Add(const HDbg& sp);
				static void Draw();
		};
	}
}
