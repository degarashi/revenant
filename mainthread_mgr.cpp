#include "mainthread.hpp"
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

namespace rev {
	void MainThread::_InitManagers(Manager& m, const GameloopParam& param) {
		m.cam2 = std::make_unique<Camera2DMgr>();
		m.cam3 = std::make_unique<Camera3DMgr>();
		m.ptrm = std::make_unique<PointerMgr>();
		m.inpm = std::make_unique<InputMgr>();
		m.info = std::make_unique<SystemInfo>();
		// 初回はここで情報をセットする (以降はIMainProc::runUにて)
		m.info->setInfo(param.getScreenSize(), 0);
		// geom = std::make_unique<util::SharedGeomM>();
		// auto				fxP(InitFxBlock());
		m.dtask = std::make_unique<draw::Task>();
		m.rwm = std::make_unique<RWMgr>(param.getOrgName(), param.getAppName());
		// デフォルトでルートディレクトリからの探索パスを追加
		m.rwm->addHandler(0x00, std::make_shared<URI_File>(u8"/"));
		m.appPath = std::make_unique<AppPath>(PathBlock(Dir::GetProgramDir()));
		// pathfile文字列が有効ならここでロードする
		if(const auto& p = param.getPathfile())
			_LoadPathfile(p);
		m.glr = std::make_unique<GLRes>();
		// m.font = std::make_unique<FontFamily>();
		// _LoadFonts();
		// m.fgen = std::make_unique<FontGen>(spn::PowSize(512,512));
		// m.scene = std::make_unique<SceneMgr>();
		m.snd = std::make_unique<SoundMgr>(44100);
		// m.urep = std::make_unique<UpdRep>();
		// m.orep = std::make_unique<ObjRep>();
		// m.lsys = std::make_unique<LSysFunc>();
		// m.objm = std::make_unique<ObjMgr>();
		m.snd->makeCurrent();
	}
	void MainThread::_LoadPathfile(const URI& uri, const bool bAppend) {
		mgr_path.setFromText(mgr_rw.fromURI(uri, Access::Read), bAppend);
	}
	// void MainThread::_LoadFonts() {
	// 	mgr_path.enumPath("font", "*.tt(c|f)", [](const spn::Dir& d){
	// 		mgr_font.loadFamily(mgr_rw.fromFile(d.plain_utf8(), RWops::Read));
	// 		return true;
	// 	});
	// }
}
