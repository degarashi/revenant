#include "mainthread.hpp"
#include "lsys.hpp"
#include "gameloopparam.hpp"
#include "systeminfo.hpp"
#include "sdl_rw.hpp"
#include "uri_file.hpp"
#include "uri_data.hpp"
#include "apppath.hpp"
#include "sound.hpp"
#include "camera2d.hpp"
#include "camera3d.hpp"
#include "input.hpp"
#include "input_sdlvalue.hpp"
#include "gl_resource.hpp"
#include "glx.hpp"
#include "font.hpp"
#include "scene_mgr.hpp"
#include "techmgr.hpp"
#include "imgui_sdl2.hpp"
#include "object_mgr.hpp"
#include "gltf/v1/mgr.hpp"

namespace rev {
	void MainThread::_InitManagers(Manager& m, const GameloopParam& param, const Window& w) {
		m.lsys = std::make_shared<LSysFunc>();
		m.inpm = std::make_shared<InputMgr>();
		m.info = std::make_shared<SystemInfo>();
		// 初回はここで情報をセットする (以降はIMainProc::runUにて)
		m.info->setInfo(param.getScreenSize(), 0);
		m.rwm = std::make_shared<RWMgr>(param.getOrgName(), param.getAppName());
		// デフォルトでルートディレクトリからの探索パスを追加
		m.rwm->addHandler(0x00, std::make_shared<URI_File>(u8"/"));
		// データURIのローダーも追加
		m.rwm->addHandler(0x01, std::make_shared<URI_Data>());
		m.appPath = std::make_shared<AppPath>(PathBlock(Dir::GetProgramDir()));
		// pathfile文字列が有効ならここでロードする
		if(const auto& p = param.getPathfile())
			_LoadPathfile(*p);
		m.glr = std::make_shared<GLRes>();
		m.glr->onDeviceReset();
		m.font = std::make_shared<FontFamily>();
		_LoadFonts();
		m.fgen = std::make_shared<FontGen>(lubee::PowSize(512,512));
		m.snd = std::make_shared<SoundMgr>(44100);
		m.snd->makeCurrent();
		m.tech = std::make_shared<parse::TechMgr>();
		m.obj = std::make_shared<ObjMgr>();
		m.scene = std::make_shared<SceneMgr>();
		m.imgui = std::make_shared<ImGui_SDL2>(
			Keyboard::OpenKeyboard(),
			Mouse::OpenMouse(0),
			w
		);
		m.gltf = std::make_shared<gltf::v1::GLTFMgr>();
	}
	void MainThread::_LoadPathfile(const URI& uri, const bool bAppend) {
		mgr_path.setFromText(mgr_rw.fromURI(uri, Access::Read), bAppend);
	}
	void MainThread::_LoadFonts() {
		mgr_path.enumPath("font", PathBlock("*.tt(c|f)"), [](const Dir& d){
			mgr_font.loadFamily(mgr_rw.fromFile(d, Access::Read|Access::Binary));
			return true;
		});
	}
}