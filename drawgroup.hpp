#pragma once
#include "drawable.hpp"
#include "drawsort.hpp"
#include "updgroup.hpp"

namespace rev {
	class DrawGroup : public DrawableObj {
		private:
			DSort_V		_dsort;			//!< ソートアルゴリズム優先度リスト
			bool		_bDynamic;		//!< 毎フレームソートするか
			DLObj_V		_dobj;

			void _doDrawSort();
			static DSort_V _MakeDSort(const SortAlgList& al);
		public:
			// 描画ソート方式を指定
			DrawGroup(const DSort_V& al, bool bDynamic=false, Priority prio=DefaultPriority);
			DrawGroup(const SortAlgList& al, bool bDynamic=false, Priority prio=DefaultPriority);
			DrawGroup(Priority prio=DefaultPriority);

			void addObj(const HDObj& hObj);
			void remObj(const HDObj& hObj);
			void clear();
			void onUpdate(bool bFirst) override;
			void setSortAlgorithm(const DSort_V& ds, bool bDynamic);
			void setSortAlgorithmId(const SortAlgList& al, bool bDynamic);
			const DSort_V& getSortAlgorithm() const noexcept;
			const DLObj_V& getMember() const noexcept;
			int getNMember() const noexcept;

			bool isNode() const noexcept override;
			void onDraw(IEffect& e) const override;
			DrawTag& refDTag() noexcept;

			const char* getResourceName() const noexcept override;
	};
	#define DefineDrawGroup(name)	DefineUpdBase(name, ::rev::DrawGroup)
	DefineDrawGroup(U_DrawGroup)
}
DEF_LUAIMPORT(rev::DrawGroup)
DEF_LUAIMPORT(rev::U_DrawGroup)
