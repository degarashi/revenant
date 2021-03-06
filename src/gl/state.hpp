#pragma once
#include <cstddef>
#include "debug_gui/if.hpp"
#include "spine/src/enum.hpp"

namespace rev {
	namespace draw {
		class IQueue;
	}
	struct GLState : IDebugGui {
		DefineEnum(Type, (Boolean)(Value));
		virtual Type getType() const noexcept = 0;

		virtual void dcmd_export(draw::IQueue& q) const = 0;
		virtual void dcmd_reset(draw::IQueue& q) const = 0;
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
