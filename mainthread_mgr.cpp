#include "mainthread.hpp"
#include "lsys.hpp"
#include "gameloopparam.hpp"
#include "systeminfo.hpp"
#include "sdl_rw.hpp"
#include "uri_file.hpp"
#include "apppath.hpp"
#include "sound.hpp"
#include "camera2d.hpp"
#include "camera3d.hpp"
#include "input.hpp"
#include "input_sdlvalue.hpp"
#include "drawtoken/task.hpp"
#include "gl_resource.hpp"
#include "glx.hpp"
#include "font.hpp"
#include "scene.hpp"

namespace rev {
	void MainThread::_InitManagers(Manager& m, const GameloopParam& param) {
		m.lsys = std::make_shared<LSysFunc>();
		m.cam2 = std::make_shared<Camera2DMgr>();
		m.cam3 = std::make_shared<Camera3DMgr>();
		m.ptrm = std::make_shared<PointerMgr>();
		m.inpm = std::make_shared<InputMgr>();
		m.info = std::make_shared<SystemInfo>();
		// 初回はここで情報をセットする (以降はIMainProc::runUにて)
		m.info->setInfo(param.getScreenSize(), 0);
		m.dtask = std::make_shared<draw::Task>();
		m.rwm = std::make_shared<RWMgr>(param.getOrgName(), param.getAppName());
		// デフォルトでルートディレクトリからの探索パスを追加
		m.rwm->addHandler(0x00, std::make_shared<URI_File>(u8"/"));
		m.appPath = std::make_shared<AppPath>(PathBlock(Dir::GetProgramDir()));
		// pathfile文字列が有効ならここでロードする
		if(const auto& p = param.getPathfile())
			_LoadPathfile(p);
		m.glr = std::make_shared<GLRes>();
		m.glr->onDeviceReset();
		m.font = std::make_shared<FontFamily>();
		_LoadFonts();
		m.fgen = std::make_shared<FontGen>(lubee::PowSize(512,512));
		m.snd = std::make_shared<SoundMgr>(44100);
		m.snd->makeCurrent();
		m.block = std::make_shared<FxBlock>();
		m.obj = std::make_shared<ObjMgr>();
		m.scene = std::make_shared<SceneMgr>();
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
