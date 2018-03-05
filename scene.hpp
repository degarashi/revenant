#pragma once
#include "object_lua.hpp"
#include "scene_base.hpp"
#include "scene_if.hpp"

namespace rev {
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
			void onUpdate(bool /*execLua*/) override final {
				// Scene自体のonUpdate()
				base::onUpdate(true);
				if(!base::isDead()) {
					UpdGroup::SetAsUpdateRoot();
					// SceneメンバのonUpdate()
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
			#define DEF_ADAPTOR(name) void name() override final { \
				base::getState()->name(base::getRef()); \
				base::_doSwitchState(); }
			DEF_ADAPTOR(onStop)
			DEF_ADAPTOR(onReStart)
			#undef DEF_ADAPTOR
	};
}
