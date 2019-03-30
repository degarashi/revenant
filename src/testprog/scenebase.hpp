#pragma once
#include "../handle/input.hpp"
#include "../handle/object.hpp"
#include "../handle/opengl.hpp"
#include "../drawutil/texthud.hpp"

namespace rev::test {
	class TestScene {
		protected:
			DefineEnum(Act,
				(Quit)
				(Pause)
			);
			mutable drawutil::TextHUD	_fps;
			HAct					_act[Act::_Num];
			HTech					_textTech;

			void _showFPS(IEffect& e) const;
			void _clearBg(IEffect& e) const;

			TestScene();
	};
}
