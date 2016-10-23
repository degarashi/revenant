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

	class ABuffer;
	using HAb = std::shared_ptr<ABuffer>;
	using WAb = std::weak_ptr<ABuffer>;

	class ASource;
	using HSs = std::shared_ptr<ASource>;
	using WSs = std::weak_ptr<ASource>;

	class AGroup;
	using HSg = std::shared_ptr<AGroup>;
	using WSg = std::weak_ptr<AGroup>;
}
