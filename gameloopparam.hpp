#pragma once
#include "lubee/size.hpp"
#include "handle.hpp"

namespace rev {
	class URI;
	using URI_SP = std::shared_ptr<URI>;
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
		virtual const URI_SP& getPathfile() const = 0;
		virtual HFx makeEffect() const = 0;
		virtual MainProc* makeMainProc() const = 0;
		virtual DrawProc* makeDrawProc() const = 0;
		virtual bool getMultiContext() const noexcept = 0;
		// ゲーム開始時、最初のSceneを作る
		virtual HScene makeFirstScene() const = 0;
	};
	using GameloopParam_UP = std::unique_ptr<GameloopParam>;
}
