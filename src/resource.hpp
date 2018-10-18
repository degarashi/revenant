#pragma once
#include "lubee/src/deep_cmp.hpp"
#include <stdexcept>

namespace rev {
	#define DEF_RESOURCE_EQUAL bool deepCmp(const Resource& r) const noexcept { return _deepCmp(*this, r); }
	struct Resource {
		virtual ~Resource() {}
		virtual const char* getResourceName() const noexcept = 0;
		template <class Self>
		bool _deepCmp(const Self& self, const Resource& r) const noexcept {
			const Self* p = dynamic_cast<const Self*>(&r);
			if(p)
				return lubee::DeepCmp()(self, *p);
			return false;
		}
		DEF_RESOURCE_EQUAL
	};
	struct ResourceNotFound : std::runtime_error {
		ResourceNotFound(const std::string& name):
			std::runtime_error("resource: " + name + "is not found")
		{}
	};
}
