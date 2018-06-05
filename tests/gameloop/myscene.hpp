#pragma once
#include "../../util/texthud.hpp"
#include "../../scene.hpp"
#include "../../handle/input.hpp"
#include "../../handle/camera.hpp"
#include "../../fppose.hpp"
#include "../../handle/model.hpp"
#include "../../dc/animation.hpp"

namespace rev::test {
	using Name = std::string;
	class MyScene : public Scene<MyScene> {
		private:
			DefineEnum(Act,
				(Quit)(Pause)(SceneSwitch)
			);
			mutable util::TextHUD	_fps;
			HAct					_act[Act::_Num];
			HObj					_gui;
			HTech					_textTech;
			std::size_t				_sceneId;

			constexpr static std::size_t NState = 3;
			struct St_Base : StateT<St_Base> {
				void onUpdate(MyScene& self) override;
				void onDraw(const MyScene& self, IEffect& e) const override;
			};
			struct St_Sprite : StateT<St_Sprite, St_Base> {
				constexpr static std::size_t NSprite = 16;
				HCam2	_camera;
				HObj	_sprite[NSprite];

				void onEnter(MyScene& self, ObjTypeId_OP) override;
				void onExit(MyScene& self, ObjTypeId_OP) override;
				void onDraw(const MyScene& self, IEffect& e) const override;
			};
			struct St_3D : StateT<St_3D, St_Base> {
				DefineEnum(Act,
					(MoveX)(MoveY)(MoveZ)
					(DirX)(DirY)(DirBtn)
				);
				HAct	_act[Act::_Num];
				HCam3	_camera;
				FPPose	_fp;
				bool	_press;

				void onEnter(MyScene& self, ObjTypeId_OP) override;
				void onUpdate(MyScene& self) override;
				void onDraw(const MyScene& self, IEffect& e) const override;
			};
			struct St_Cube : StateT<St_Cube, St_3D> {
				HDObj	_cube;
				void onEnter(MyScene& self, ObjTypeId_OP) override;
				void onExit(MyScene& self, ObjTypeId_OP) override;
			};
			struct St_glTF;
			void _setSceneById(std::size_t id);
			void _makeGui();
			void _checkQuit();
			void _checkPause();
			void _checkSwitch();
			void _showFPS(IEffect& e) const;
			void _clearBg(IEffect& e) const;

		public:
			MyScene();
	};
}
