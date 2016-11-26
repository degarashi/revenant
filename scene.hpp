#pragma once
#include "updater_lua.hpp"
#include "drawable.hpp"
#include "spine/singleton.hpp"

namespace rev {
	class IScene : public Object {
		public:
			virtual HGroup getUpdGroup() const;
			virtual HDGroup getDrawGroup() const;
	};
	#define mgr_scene (::rev::SceneMgr::ref())
	//! シーンスタックを管理
	class SceneMgr : public spi::Singleton<SceneMgr> {
		private:
			using StStack = std::vector<HScene>;
			StStack		_scene;
			//! シーンを切り替えや差し替えオペレーションがあるかのフラグ
			bool		_scOp = false;
			int			_scNPop;
			HScene		_scNext;
			LCValue		_scArg;

			void _doSceneOp();

		public:
			bool isEmpty() const noexcept;
			//! シーンスタック中のSceneBaseを取得
			IScene& getSceneInterface(int n=0) const;
			//! ヘルパー関数: シーンスタック中のUpdGroupを取得
			/*! *getSceneBase(n).update->get() と同等 */
			UpdGroup& getUpdGroupRef(int n=0) const;
			//! ヘルパー関数: シーンスタック中のDrawGroupを取得
			/*! *getSceneBase(n).draw->get() と同等 */
			DrawGroup& getDrawGroupRef(int n=0) const;
			//! getScene(0)と同義
			HScene getTop() const;
			HScene getScene(int n=0) const;
			void setPushScene(const HScene& hSc, bool bPop=false);
			void setPopScene(int nPop, const LCValue& arg=LCValue());
			//! フレーム更新のタイミングで呼ぶ
			bool onUpdate();
			//! 描画のタイミングで呼ぶ
			void onDraw(IEffect& e);
			bool onPause();
			void onEffectReset();
			void onStop();
			void onResume();
			void onReStart();
	};

	class SceneBase {
		private:
			DefineUpdGroup(Update)
			DefineDrawGroup(Draw)

			HGroup		_update;
			HDGroup		_draw;

		public:
			SceneBase(const HGroup& hUpd, const HDGroup& hDraw);
			void setUpdate(const HGroup& hGroup);
			const HGroup& getUpdate() const noexcept;
			void setDraw(const HDGroup& hDGroup);
			const HDGroup& getDraw() const noexcept;
	};
	//! 1シーンにつきUpdateTreeとDrawTreeを1つずつ用意
	template <class T>
	class Scene : public ObjectT_Lua<T, IScene> {
		private:
			using base = ObjectT_Lua<T, IScene>;
			SceneBase	_sbase;
		public:
			Scene(const HGroup& hUpd=HGroup(), const HDGroup& hDraw=HDGroup()):
				_sbase(hUpd, hDraw)
			{}
			void onUpdate(bool /*bFirst*/) override final {
				base::onUpdate(true);
				if(!base::isDead()) {
					UpdGroup::SetAsUpdateRoot();
					_sbase.getUpdate()->onUpdate(true);
				}
			}
			void onDraw(IEffect& e) const override final {
				base::onDraw(e);
				_sbase.getDraw()->onDraw(e);
			}
			void onDisconnected(const HGroup& h) override final {
				D_Assert0(!h);
				_sbase.getUpdate()->onDisconnected(h);
				base::onDisconnected(h);
			}
			void onConnected(const HGroup& h) override final {
				base::onConnected(h);
				D_Assert0(!h);
				_sbase.getUpdate()->onConnected(h);
			}
			//! ヘルパー関数: シーンスタック中のUpdGroupを取得
			/*! *getBase().update と同等 */
			UpdGroup& getUpdGroupRef() const {
				return *getBase().getUpdate();
			}
			HGroup getUpdGroup() const override {
				return getBase().getUpdate();
			}
			//! ヘルパー関数: シーンスタック中のDrawGroupを取得
			/*! *getBase().draw と同等 */
			DrawGroup& getDrawGroupRef() const {
				return *getBase().getDraw();
			}
			HDGroup getDrawGroup() const override {
				return getBase().getDraw();
			}
			const SceneBase& getBase() const {
				return _sbase;
			}
			SceneBase& getBase() {
				return _sbase;
			}
			bool onPause() override final {
				return base::onPause();
			}
			void onResume() override final {
				base::onResume();
			}
			void onEffectReset() override final {
				base::onEffectReset();
			}
			#define DEF_ADAPTOR(name) void name() override final { \
				base::getState()->name(base::getRef()); \
				base::_doSwitchState(); }
			DEF_ADAPTOR(onStop)
			DEF_ADAPTOR(onReStart)
			#undef DEF_ADAPTOR
	};
	class U_Scene : public Scene<U_Scene> {
		private:
			struct St_None;
		public:
			U_Scene();
	};
}
