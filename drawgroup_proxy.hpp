#pragma once
#include "drawable.hpp"
#include "updgroup.hpp"

namespace rev {
	struct DSort;
	using DSort_SP = std::shared_ptr<DSort>;
	using DSort_V = std::vector<DSort_SP>;
	using DLObj_P = std::pair<const DrawTag*, HDObj>;
	using DLObj_V = std::vector<DLObj_P>;
	class DrawGroupProxy : public DrawableObj {
		private:
			HDGroup		_hDGroup;
		public:
			DrawGroupProxy(const HDGroup& hDg, Priority prio=DefaultPriority);

			void onUpdate(bool execLua) override;
			const DSort_V& getSortAlgorithm() const noexcept;
			const DLObj_V& getMember() const noexcept;

			void setPriority(Priority p) noexcept;
			bool isNode() const noexcept override;
			void onDraw(IEffect& e) const override;
			ObjTypeId getTypeId() const override;
			DrawTag& refDTag() noexcept;

			const char* getResourceName() const noexcept override;
	};
}
DEF_LUAIMPORT(rev::DrawGroupProxy)
