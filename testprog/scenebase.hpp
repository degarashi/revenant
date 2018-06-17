#pragma once
#include "../handle/input.hpp"
#include "../handle/object.hpp"
#include "../util/texthud.hpp"

namespace rev::test {
	class TestScene {
		protected:
			DefineEnum(Act,
				(Quit)
				(Pause)
			);
			mutable util::TextHUD	_fps;
			HAct					_act[Act::_Num];
			HTech					_textTech;

			void _showFPS(IEffect& e) const;
			void _clearBg(IEffect& e) const;

			TestScene();
	};
}
