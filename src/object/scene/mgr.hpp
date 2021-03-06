#pragma once
#include "../../lua/lcvalue.hpp"
#include "../../debug_gui/if.hpp"
#include "../../handle/object.hpp"
#include "spine/src/singleton.hpp"
#include <vector>

namespace rev {
	class IScene;
	class IEffect;
	#define mgr_scene (::rev::SceneMgr::ref())
	//! シーンスタックを管理
	class SceneMgr :
		public IDebugGui,
		public spi::Singleton<SceneMgr>
	{
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
			IUpdGroup& getUpdGroupRef(int n=0) const;
			//! ヘルパー関数: シーンスタック中のDrawGroupを取得
			/*! *getSceneBase(n).draw->get() と同等 */
			DrawGroup& getDrawGroupRef(int n=0) const;
			//! getScene(0)と同義
			HScene getTop() const;
			HScene getScene(int n=0) const;
			void setPushScene(const HScene& hSc, bool bPop=false);
			void setPopScene(int nPop, const LCValue& arg=LCValue());
			//! フレーム更新のタイミングで呼ぶ
			/*! \return スタックが空の場合はfalseを返す */
			bool onUpdate();
			//! 描画のタイミングで呼ぶ
			void onDraw(IEffect& e);
			bool onPause();
			void onStop();
			void onResume();
			void onReStart();
			DEF_DEBUGGUI_PROP
	};
}
#include "lua/import.hpp"
DEF_LUAIMPORT(rev::SceneMgr)
