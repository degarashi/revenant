#pragma once
#include <memory>

namespace rev {
	struct IDrawProc {
		//! 描画コールバック
		/*!
			描画スレッドから呼ばれる
			\param[in] accum 累積フレーム数
			\param[in] bSkip 描画スキップフラグ
			\return backbufferのswapをかける時はtrue
		*/
		virtual bool runU(uint64_t accum, bool bSkip) = 0;
		virtual ~IDrawProc() {}
	};
	using DrawProc_UP = std::unique_ptr<IDrawProc>;
}
