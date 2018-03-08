#pragma once
#include "updgroup.hpp"
#include "interval.hpp"

namespace rev {
	//! UpdGroupにフレームカウンタやアイドル機能をつけたクラス
	class UpdTask : public UpdGroup {
		private:
			using Interval = interval::Combine<interval::Wait>;
			Interval	_interval;

		public:
			UpdTask(Priority p=DefaultPriority);

			void onUpdate(bool execLua) override;
			/*!	\param[in] nFrame	待機フレーム数(負数は無限) */
			void setIdle(int nFrame);
			int getIdle() const noexcept;
			int getAccum() const noexcept;
	};
}
