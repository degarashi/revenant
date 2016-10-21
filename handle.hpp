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
		#undef DEF_NAME
	}
	struct ResourceNotFound : std::runtime_error {
		ResourceNotFound(const std::string& name):
			std::runtime_error("resource: " + name + "is not found")
		{}
	};

	class RWops;
	using HRW = std::shared_ptr<RWops>;
	using WRW = std::weak_ptr<RWops>;

	class Action;
	using HAct = std::shared_ptr<Action>;
	using WAct = std::weak_ptr<Action>;

	struct Input;
	using HInput = std::shared_ptr<Input>;
	using WInput = std::weak_ptr<Input>;

	class Action;
	using HAct = std::shared_ptr<Action>;
	using WAct = std::weak_ptr<Action>;

	struct TPos2D;
	using HPtr = std::shared_ptr<TPos2D>;
	using WPtr = std::weak_ptr<TPos2D>;
}
