#pragma once
#include "object.hpp"
#include "sdl_tls.hpp"
#include <vector>

namespace rev {
	//! Objectのグループ管理
	class UpdGroup : public Object, public std::enable_shared_from_this<UpdGroup> {
		private:
			static TLS<bool> tls_bUpdateRoot;
			using UGVec = std::vector<WGroup>;
			//! Add or Remove予定のオブジェクトを持っているグループを登録
			static UGVec	s_ug;

			using ObjV = std::vector<HObj>;
			using ObjVP = std::vector<std::pair<Priority, HObj>>;
			using GroupV = std::vector<HGroup>;

			Priority	_priority;
			ObjVP		_objV;		//!< 優先度は実行中に変わることはないのでキャッシュしておく
			ObjVP		_addObj;	//!< (onUpdateが終った後に)追加予定のオブジェクト
			ObjV		_remObj;	//!< (onUpdateが終った後に)削除予定のオブジェクト
			GroupV		_groupV;	//!< Idでグループ検索する時用
			int			_nParent;

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

			//! オブジェクト又はグループを追加
			void addObj(const HObj& hObj);
			void addObjPriority(const HObj& hObj, Priority p);
			//! オブジェクト又はグループを削除
			void remObj(const HObj& hObj);

			bool isNode() const override;
			void onConnected(const HGroup& hGroup) noexcept override;
			void onDisconnected(const HGroup& hGroup) override;
			void onUpdate(bool execLua) override;
			void onDraw(IEffect& e) const override;
			void enumGroup(const CBFindGroup& cb, GroupTypeId id, int depth) const override;
			//! グループ内のオブジェクト全てに配信
			bool recvMsg(LCValue& dst, const GMessageStr& msg, const LCValue& arg) override;
			void proc(const CBUpdProc& p, bool bRecursive, Priority prioBegin, Priority prioEnd) override;

			const ObjVP& getList() const noexcept;
			ObjVP& getList() noexcept;
			int getNMember() const noexcept;
			//! 子要素を全て削除
			void clear();

			const char* getResourceName() const noexcept override;
	};
	namespace idtag {
		struct Group {};
	}
	template <class T, class Base>
	class GroupT : public Base, public ObjectIdT<T, idtag::Group> {
		private:
			using IdT = ObjectIdT<T, idtag::Group>;
		public:
			using Base::Base;
			ObjTypeId getTypeId() const override { return GetTypeId(); }
			static ObjTypeId GetTypeId() { return IdT::Id; }
	};
	#define DefineUpdBase(name, base) \
		class name : public ::rev::GroupT<name, base> { \
			private: \
				using base_t = ::rev::GroupT<name, base>; \
			public: \
				using base_t::base_t; \
		};
	#define DefineUpdGroup(name) DefineUpdBase(name, ::rev::UpdGroup)
	DefineUpdGroup(U_UpdGroup)
}
DEF_LUAIMPORT(rev::UpdGroup)
DEF_LUAIMPORT(rev::U_UpdGroup)
