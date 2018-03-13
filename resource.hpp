#pragma once
#include "lubee/deep_cmp.hpp"

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
}
