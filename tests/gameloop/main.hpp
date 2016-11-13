#pragma once
#include "../test.hpp"
#include "../../drawproc.hpp"
#include "../../mainproc.hpp"
#include "../../gameloopparam.hpp"

namespace rev {
	namespace test {
		struct DrawProc : IDrawProc {
			bool runU(uint64_t accum, bool bSkip) override;
		};
		struct MainProc : IMainProc {
			HAct _actQ;

			MainProc();
			bool runU(Query& q) override;
			bool onPause() override;
			void onResume() override;
			void onStop() override;
			void onReStart() override;
		};
		struct Param : GameloopParam {
			lubee::SizeI getScreenSize() const override;
			//! アプリケション名(一時ファイル用)
			const std::string& getAppName() const override;
			//! 組織名(一時ファイル用)
			const std::string& getOrgName() const override;
			//! パス記述ファイル名
			const URI& getPathfile() const override;
			HFx loadEffect(const std::string& name) const override;
			IMainProc* makeMainProc(const Window_SP&) const override;
			IDrawProc* makeDrawProc() const override;
			bool getMultiContext() const noexcept override;
		};
		struct GameloopTest : Random {};
	}
}
