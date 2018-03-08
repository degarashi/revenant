#pragma once
#include "updgroup_if.hpp"
#include "sdl_tls.hpp"

namespace rev {
	//! Objectのグループ管理
	class UpdGroup :
		public IUpdGroup,
		public std::enable_shared_from_this<UpdGroup>
	{
		private:
			static TLS<bool> tls_bUpdateRoot;
			using UGVec = std::vector<std::weak_ptr<UpdGroup>>;
			//! Add or Remove予定のオブジェクトを持っているグループを登録
			static UGVec	s_ug;

			using GroupV = std::vector<HGroup>;

			Priority	_priority;
			ObjVP		_objV;		//!< 優先度は実行中に変わることはないのでキャッシュしておく
			ObjVP		_addObj;	//!< (onUpdateが終った後に)追加予定のオブジェクト
			ObjV		_remObj;	//!< (onUpdateが終った後に)削除予定のオブジェクト
			GroupV		_groupV;	//!< Idでグループ検索する時用
			int			_nParent;

			using Interval = interval::Combine<interval::Wait, interval::EveryN>;
			Interval	_interval;

			//! Add,RemoveListが空なら自身をグローバルリストに登録
			void _registerUGVec();
			//! オブジェクト又はグループを実際に追加、削除
			/*! onUpdate内で暗黙的に呼ばれる */
			void _doAddRemove();

			//! データの接合性チェックをメンバ変更のある全てのUpdGroupに対して行う
			static void _AllCheck();
			void _checkInternalData() const;

			template <class T>
			friend class Scene;
			friend class SceneMgr;
			static void ProcAddRemove();
		public:
			static void SetAsUpdateRoot();
			UpdGroup(Priority p=DefaultPriority);
			~UpdGroup();
			Priority getPriority() const override;

			interval::Wait& refWait() override;
			interval::EveryN& refEveryN() override;

			bool isNode() const noexcept override;
			ObjTypeId getTypeId() const override;
			void onConnected(const HGroup& hGroup) override;
			void onDisconnected(const HGroup& hGroup) override;
			void onUpdate(bool execLua) override;
			void onDraw(IEffect& e) const override;
			void enumGroup(const CBFindGroup& cb, GroupTypeId id, int depth) const override;
			//! グループ内のオブジェクト全てに配信
			bool recvMsg(LCValue& dst, const GMessageStr& msg, const LCValue& arg) override;
			void proc(const CBUpdProc& p, bool bRecursive, Priority prioBegin, Priority prioEnd) override;

			//! オブジェクト又はグループを追加
			void addObj(const HObj& hObj) override;
			void addObjPriority(const HObj& hObj, Priority p) override;
			//! オブジェクト又はグループを削除
			void remObj(const HObj& hObj) override;
			const ObjVP& getList() const noexcept override;
			ObjVP& getList() noexcept override;
			int getNMember() const noexcept override;
			//! 子要素を全て削除
			void clear() override;
			HGroup getHandle() override;

			const char* getResourceName() const noexcept override;
	};
}
DEF_LUAIMPORT(rev::UpdGroup)
