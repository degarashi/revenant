#pragma once
#include "updgroup.hpp"

namespace rev {
	//! UpdGroupにフレームカウンタやアイドル機能をつけたクラス
	/*! 中身は別のUpdGroupを使用 */
	class UpdTask : public Object {
		private:
			int			_idleCount,		//!< 再起動までの待ち時間
						_accum;			//!< 累積フレーム数
			HGroup		_hGroup;		//!< 参照しているグループハンドル

		public:
			UpdTask(Priority p, const HGroup& hGroup);

			bool isNode() const noexcept override;
			void onConnected(const HGroup& hGroup) override;
			void onDisconnected(const HGroup& hGroup) override;
			void enumGroup(const CBFindGroup& cb, GroupTypeId id, int depth) const override;
			bool recvMsg(LCValue& dst, const GMessageStr& msg, const LCValue& arg) override;
			void proc(const CBUpdProc& p, bool bRecursive, Priority prioBegin, Priority prioEnd) override;

			void onUpdate(bool execLua) override;
			/*!	\param[in] nFrame	待機フレーム数(負数は無限) */
			void setIdle(int nFrame);
			int getAccum() const;
			UpdTask& operator = (const UpdTask&) = delete;

			const char* getResourceName() const noexcept override;
	};
}
