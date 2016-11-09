#pragma once
#include "clock.hpp"
#include <memory>

namespace rev {
	namespace detail {
		extern const bool c_pauseDefault;
	}
	struct IMainProc {
		class Query {
			private:
				bool		_bDraw;
				Timepoint	_tp;
				int			_skip;
			public:
				Query(Timepoint tp, int skip);
				bool canDraw() const;
				void setDraw(bool bDraw);
				bool getDraw() const;
		};

		//! 毎フレームのアップデート処理
		/*! \return falseを返すとアプリケーション終了 */
		virtual bool runU(Query& q) = 0;
		/*! \return trueを返すとゲーム進行を一時停止する */
		virtual bool onPause();
		virtual void onResume();
		virtual void onStop();
		virtual void onReStart();

		virtual ~IMainProc() {}
	};
	using MainProc_UP = std::unique_ptr<IMainProc>;
}
