#pragma once
#include <memory>

namespace rev {
	struct Resource {
		virtual ~Resource() {}
		virtual const char* getResourceName() const noexcept = 0;
	};
	namespace resource_name {
		#define DEF_NAME(name) constexpr const char* name = #name
		DEF_NAME(RWops);
		DEF_NAME(TPos2D);
		DEF_NAME(Action);
		DEF_NAME(Input);
		DEF_NAME(ABuffer);
		DEF_NAME(ASource);
		DEF_NAME(AGroup);
		#undef DEF_NAME
	}
	struct ResourceNotFound : std::runtime_error {
		ResourceNotFound(const std::string& name):
			std::runtime_error("resource: " + name + "is not found")
		{}
	};

	#define DEF_HANDLE(typ, name) \
		using H##name = std::shared_ptr<typ>; \
		using W##name = std::weak_ptr<typ>;
	DEF_HANDLE(Resource, Res)

	class RWops;
	DEF_HANDLE(RWops, RW)

	struct Input;
	DEF_HANDLE(Input, Input)
	class Action;
	DEF_HANDLE(Action, Act)
	struct TPos2D;
	DEF_HANDLE(TPos2D, Ptr)
	class ABuffer;
	DEF_HANDLE(ABuffer, Ab)
	class ASource;
	DEF_HANDLE(ASource, Ss)
	class AGroup;
	DEF_HANDLE(AGroup, Sg)

	#undef DEF_HANDLE
}
