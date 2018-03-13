#include "resource_window.hpp"
#include "../resource.hpp"
#include "window.hpp"
#include "sstream.hpp"
#include "../imgui/imgui.h"
#include <iomanip>
#include "../debuggui_if.hpp"

namespace rev {
	namespace debug {
		ResourceWindow::Entry::Entry(const void *const p, WP wp):
			ptr(p),
			wp(wp)
		{}
		bool ResourceWindow::Entry::operator == (const Entry& e) const noexcept {
			return ptr == e.ptr;
		}
		std::size_t ResourceWindow::Entry::operator ()(const Entry& e) const noexcept {
			return std::hash<const void*>()(e.ptr);
		}

		ResourceWindow::Set ResourceWindow::s_set;
		void ResourceWindow::Add(const SP& sp) {
			s_set.emplace(sp.get(), sp);
		}
		void ResourceWindow::Draw() {
			StringStream s;
			s << std::hex;
			auto itr = s_set.begin();
			while(itr != s_set.end()) {
				if(const auto sp = itr->wp.lock()) {
					s << sp.get() << '\t' << sp->getDebugName();
					bool opened = true;
					if(const auto w = WindowPush(s.output().c_str(), &opened, 640,480)) {
						ImGui::Text("reference-count = %ld", sp.use_count());
						sp->property(true);
					}
					if(!opened) {
						itr = s_set.erase(itr);
					} else
						++itr;
				} else {
					itr = s_set.erase(itr);
				}
			}
		}
	}
}
