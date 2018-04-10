#pragma once
#include "../../gameloopparam.hpp"

namespace rev::test {
	struct Param : GameloopParam {
		lubee::SizeI getScreenSize() const override;
		//! アプリケション名(一時ファイル用)
		const std::string& getAppName() const override;
		//! 組織名(一時ファイル用)
		const std::string& getOrgName() const override;
		//! パス記述ファイル名
		const HURI& getPathfile() const override;
		HFx makeEffect() const override;
		::rev::MainProc* makeMainProc() const override;
		::rev::DrawProc* makeDrawProc() const override;
		bool getMultiContext() const noexcept override;
		HScene makeFirstScene() const override;
	};
}
