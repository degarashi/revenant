#pragma once
#include <memory>

namespace rev {
	struct DrawProc {
		//! 描画コールバック
		/*!
			描画スレッドから呼ばれる
			\param[in] accum 累積フレーム数
			\return backbufferのswapをかける時はtrue
		*/
		virtual bool runU(uint64_t accum);
		virtual ~DrawProc() {}
	};
	using DrawProc_UP = std::unique_ptr<DrawProc>;
}
