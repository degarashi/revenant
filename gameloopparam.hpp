#pragma once
#include "lubee/size.hpp"
#include "handle.hpp"

namespace rev {
	class URI;
	class Window;
	using Window_SP = std::shared_ptr<Window>;

	struct MainProc;
	struct DrawProc;
	struct GameloopParam {
		//! 画面解像度
		virtual lubee::SizeI getScreenSize() const = 0;
		//! アプリケション名(一時ファイル用)
		virtual const std::string& getAppName() const = 0;
		//! 組織名(一時ファイル用)
		virtual const std::string& getOrgName() const = 0;
		//! パス記述ファイル名
		virtual const URI& getPathfile() const = 0;
		virtual HFx loadEffect(const std::string& name) const = 0;
		virtual HFx makeDefaultEffect() const = 0;
		virtual MainProc* makeMainProc() const = 0;
		virtual DrawProc* makeDrawProc() const = 0;
		virtual bool getMultiContext() const noexcept = 0;
		// 最初のSceneを作る
		// virtual void makeScene(const HObj& obj) const = 0;
	};
	using GameloopParam_UP = std::unique_ptr<GameloopParam>;
}
