#pragma once
#include "drawtag.hpp"
#include "updater.hpp"
#include "drawsort.hpp"

namespace rev {
	class DrawableObj : public Object {
		protected:
			// ソート用の描画タグ
			DrawTag		_dtag;
		public:
			DrawableObj(Priority prio=DefaultPriority);
			const DrawTag& getDTag() const noexcept;
			void setDrawPriority(Priority p) noexcept;
	};
	// Object::PriorityはUpdateObjと兼用の場合に使われる
	template <class T>
	using DrawableObjT = ObjectT<T, DrawableObj>;

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
	class DrawGroupProxy : public DrawableObj {
		private:
			HDGroup		_hDGroup;
		public:
			DrawGroupProxy(const HDGroup& hDg, Priority prio=DefaultPriority);

			void onUpdate(bool bFirst) override;
			const DSort_V& getSortAlgorithm() const noexcept;
			const DLObj_V& getMember() const noexcept;

			void setPriority(Priority p) noexcept;
			bool isNode() const noexcept override;
			void onDraw(IEffect& e) const override;
			DrawTag& refDTag() noexcept;

			const char* getResourceName() const noexcept override;
	};
	#define DefineDrawGroup(name)	DefineUpdBase(name, ::rev::DrawGroup)
	#define DefineDrawGroupProxy(name)	DefineUpdBase(name, ::rev::DrawGroupProxy)

	DefineDrawGroup(U_DrawGroup)
	DefineDrawGroupProxy(U_DrawGroupProxy)
}
DEF_LUAIMPORT(rev::DrawableObj)
DEF_LUAIMPORT(rev::DrawGroup)
DEF_LUAIMPORT(rev::DrawGroupProxy)
DEF_LUAIMPORT(rev::U_DrawGroup)
DEF_LUAIMPORT(rev::U_DrawGroupProxy)
