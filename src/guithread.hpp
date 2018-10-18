#pragma once
#include "spine/src/singleton.hpp"
#include "spine/src/enum.hpp"
#include "spine/src/optional.hpp"
#include "handler.hpp"

union SDL_Event;
namespace rev {
	struct GameloopParam;
	using GameloopParam_UP = std::unique_ptr<GameloopParam>;
	//! ゲームループメイン(GUIスレッド)
	class GUIThread : public spi::Singleton<GUIThread> {
		private:
			DefineEnum(
				Level,
				/* ゲーム停止。リソース解放
					Android: OnPauseの時
					Desktop: 最小化された時 */
				(Stop)
				/* ゲーム一時停止。リソースは開放しない
					Desktop: フォーカスが外れた時 */
				(Pause)
				// ゲーム進行中
				(Active)
			);
			void _onPause();
			void _onResume();
			void _onStop();
			void _onReStart();

			using LFunc = void (GUIThread::*)();
			const static LFunc cs_lfunc[Level::_Num][2];
			void _setLevel(Level level);
			void _procWindowEvent(SDL_Event& e);
			void _procMouseWheel(SDL_Event& e);
			void _procMouseButtonDown(SDL_Event& e);
			void _procKey(SDL_Event& e);
			void _procTextInput(SDL_Event& e);

			using Handler_OP = spi::Optional<Handler>;
			Handler_OP			_handler;
			Level				_level;

		public:
			GUIThread(GameloopParam_UP param);
			/*! \retval 0		正常に終了
				\retval 0以外	異常終了 */
			int run();
	};
}
