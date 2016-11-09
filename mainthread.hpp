#pragma once
#include "spine/singleton.hpp"
#include "looper_thread.hpp"
#include "mainproc.hpp"

namespace rev {
	struct GameloopParam;
	class URI;

	class Camera2DMgr;
	class Camera3DMgr;
	class PointerMgr;
	class InputMgr;
	class SystemInfo;
	namespace draw {
		class Task;
	}
	class RWMgr;
	class AppPath;
	class GLRes;
	class SoundMgr;
	//! メインスレッド
	class MainThread : public spi::Singleton<MainThread>,
						public ThreadL<void (const Looper_SP&)>
	{
		private:
			struct Info {
				uint64_t	accumUpd;	//!< アップデート累積カウンタ
				uint64_t	accumDraw;	//!< 描画フレーム累積カウンタ
				Timepoint	tmBegin;	//!< ゲーム開始時の時刻
			};
			SpinLock<Info>		_info;

			template <class T>
			using SPtr = std::shared_ptr<T>;
			struct Manager : spi::Singleton<Manager> {
				SPtr<Camera2DMgr>			cam2;
				SPtr<Camera3DMgr>			cam3;
				SPtr<PointerMgr>			ptrm;
				SPtr<InputMgr>				inpm;
				SPtr<SystemInfo>			info;
				SPtr<draw::Task>			dtask;
				SPtr<RWMgr>					rwm;
				SPtr<AppPath>				appPath;
				SPtr<GLRes>					glr;
				SPtr<SoundMgr>				snd;
				// SPtr<util::SharedGeomM>		geom;
				// SPtr<FontFamily>			font;
				// SPtr<FontGen>			fgen;
				// SPtr<SceneMgr>			scene;
				// SPtr<UpdRep>				urep;
				// SPtr<ObjRep>				orep;
				// SPtr<LSysFunc>			lsys;
				// SPtr<ObjMgr>				objm;
			};
			static void _InitManagers(Manager& m, const GameloopParam& param);
			//! AppPathの場所に置かれたフォントファイルを列挙し、読み込む
			// static void _LoadFonts();
			//! AppPathをテキストファイルから設定
			static void _LoadPathfile(const URI& uri, bool bAppend=false);
			static void _Backup(Manager& m, std::ostream& os);
			static void _Restore(Manager& m, std::istream& is);
		protected:
			void runL(const Looper_SP& guiLooper) override;
		public:
			MainThread();
			decltype(auto) getInfo() { return _info.lock(); }
			decltype(auto) getInfo() const { return _info.lockC(); }
	};
}