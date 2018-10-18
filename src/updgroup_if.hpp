#pragma once
#include "object_if.hpp"
#include "interval.hpp"
#include <vector>

namespace rev {
	class IUpdGroup : public IObject {
		protected:
			using ObjV = std::vector<HObj>;
			using ObjVP = std::vector<std::pair<Priority, HObj>>;

		public:
			virtual void addObj(const HObj& hObj) = 0;
			virtual void addObjPriority(const HObj& hObj, Priority p) = 0;
			virtual void remObj(const HObj& hObj) = 0;
			virtual const ObjVP& getList() const noexcept = 0;
			virtual ObjVP& getList() noexcept = 0;
			virtual int getNMember() const noexcept = 0;
			virtual void clear() = 0;
			virtual HGroup getHandle() = 0;
			virtual interval::Wait& refWait() = 0;
			virtual interval::EveryN& refEveryN() = 0;
	};
}
