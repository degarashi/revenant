#pragma once
#include "spine/singleton.hpp"
#include "looper_thread.hpp"
#include "mainproc.hpp"
#include "dir.hpp"
#include "spinlock.hpp"
#include "handle/opengl.hpp"
#include <unordered_set>

namespace rev {
	struct GameloopParam;
	class URI;

	class Window;
	class DrawThread;
	class FNotify;
	class InputMgr;
	class SystemInfo;
	namespace draw {
		class Task;
	}
	class LSysFunc;
	class RWMgr;
	class AppPath;
	class GLRes;
	class SoundMgr;
	class FontFamily;
	class FontGen;
	struct UnifPool;
	namespace parse {
		class FxBlock;
	}
	class ObjMgr;
	class SceneMgr;
	class ImGui_SDL2;
	namespace gltf {
		class GLTFMgr;
	}
	//! メインスレッド
	class MainThread : public spi::Singleton<MainThread>,
						public ThreadL<void (const Looper_SP&)>
	{
		private:
			struct Info {
				uint64_t	accumUpd;	//!< アップデート累積カウンタ
				uint64_t	accumDraw;	//!< 描画フレーム累積カウンタ
			};
			SpinLock<Info>		_info;

			template <class T>
			using SPtr = std::shared_ptr<T>;
			struct Manager : spi::Singleton<Manager> {
				SPtr<LSysFunc>				lsys;
				SPtr<InputMgr>				inpm;
				SPtr<SystemInfo>			info;
				SPtr<AppPath>				appPath;
				SPtr<GLRes>					glr;
				SPtr<draw::Task>			dtask;
				SPtr<SoundMgr>				snd;
				SPtr<RWMgr>					rwm;
				SPtr<FontFamily>			font;
				SPtr<FontGen>				fgen;
				SPtr<parse::FxBlock>		block;
				SPtr<UnifPool>				unifPool;
				SPtr<ObjMgr>				obj;
				SPtr<SceneMgr>				scene;
				SPtr<ImGui_SDL2>			imgui;
				SPtr<gltf::GLTFMgr>			gltf;
			};
			struct FxReload {
				HFx	curFx, prevFx;
				int	prevFxCounter = 0;
				using PathSet = std::unordered_set<Dir>;
				PathSet updatePath;
			};
			static void _InitManagers(Manager& m, const GameloopParam& param, const Window& w);
			//! AppPathの場所に置かれたフォントファイルを列挙し、読み込む
			static void _LoadFonts();
			//! AppPathをテキストファイルから設定
			static void _LoadPathfile(const URI& uri, bool bAppend=false);
			static void _Backup(Manager& m, std::ostream& os);
			static void _Restore(Manager& m, std::istream& is);
			void _setupFxNotify(FNotify& ntf);
			void _checkFxReload(FNotify& ntf, FxReload& rel);
			static Timepoint _WaitForNextInterval(Timepoint prevtime, Duration interval);
			bool _updateFrame(MainProc* mp, DrawThread& dth, Handler& drawHandler, Duration delta);
		protected:
			void runL(const Looper_SP& guiLooper) override;
		public:
			MainThread();
			decltype(auto) getInfo() { return _info.lock(); }
			decltype(auto) getInfo() const { return _info.lockC(); }
	};
}
