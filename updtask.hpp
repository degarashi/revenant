#pragma once
#include "updgroup.hpp"

namespace rev {
	//! UpdGroupにフレームカウンタやアイドル機能をつけたクラス
	/*! 中身は別のUpdGroupを使用 */
	class UpdTask : public UpdGroup {
		private:
			int			_idleCount,		//!< 再起動までの待ち時間
						_accum;			//!< 累積フレーム数

		public:
			UpdTask(Priority p=DefaultPriority);

			void onUpdate(bool execLua) override;
			/*!	\param[in] nFrame	待機フレーム数(負数は無限) */
			void setIdle(int nFrame);
			int getIdle() const noexcept;
			int getAccum() const noexcept;
	};
}
