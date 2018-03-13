#pragma once
#include <cstddef>
#include "debuggui_if.hpp"
#include "spine/enum.hpp"

namespace rev {
	struct GLState : IDebugGui {
		DefineEnum(Type, (Boolean)(Value));
		virtual Type getType() const noexcept = 0;
		virtual void apply() const = 0;

		virtual bool operator == (const GLState& s) const noexcept = 0;
		virtual std::size_t getHash() const noexcept = 0;
		template <class Self, class Other>
		static bool _Compare(const Self& self, const Other& other) noexcept {
			if(auto* p = dynamic_cast<const Self*>(&other))
				return self == *p;
			return false;
		}
		bool operator != (const GLState& s) const noexcept {
			return !(this->operator == (s));
		}
	};
}
