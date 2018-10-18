#pragma once
#include "clock.hpp"
#include <memory>

namespace rev {
	namespace detail {
		extern const bool c_pauseDefault;
	}
	struct MainProc {
		//! 毎フレームのアップデート処理
		/*! \return falseを返すとアプリケーション終了 */
		virtual bool runU(Duration delta);
		/*! \return trueを返すとゲーム進行を一時停止する */
		virtual bool onPause();
		virtual void onResume();
		virtual void onStop();
		virtual void onReStart();

		virtual ~MainProc() {}
	};
	using MainProc_UP = std::unique_ptr<MainProc>;
}
