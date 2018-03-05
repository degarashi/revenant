#pragma once
#include "spine/singleton.hpp"
#include "lcvalue.hpp"

namespace rev {
	class IScene;
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
	};
}
#include "luaimport.hpp"
DEF_LUAIMPORT(rev::SceneMgr)
