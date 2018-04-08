#pragma once
#include "../../gameloopparam.hpp"
#include "../../spinlock.hpp"
#include "../../util/texthud.hpp"
#include "../../scene.hpp"
#include "../../handle/input.hpp"
#include "../../handle/camera.hpp"

namespace rev {
	namespace test {
		struct UserShare {
			HTech	textTech;
		};
		extern SpinLock<UserShare> g_shared;

		struct MyScene : Scene<MyScene> {
			util::TextHUD	_fps;
			HAct			_actQ,
							_actPause;
			HCam2			_camera;

			struct St_None;
			MyScene();
			~MyScene();
		};
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
}
