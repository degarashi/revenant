#pragma once
#include "uniform_setter.hpp"
#include "lubee/src/size.hpp"

namespace rev {
	//! (3D/2D共通)
	/*!
		予め変数名がsys_*** の形で決められていて, 存在すれば計算&設定される
		固定変数リスト:
		vec4 sys_screen;		// x=ScreenSizeX, y=ScreenSizeY, z=1DotSizeX, w=1DotSizeY
	*/
	class SystemUniform : public IUniformSetter {
		private:
			lubee::SizeI	_screenSize;
		public:
			const lubee::SizeI& getScreenSize() const;
			void setScreenSize(const lubee::SizeI& s);
			UniformSetF getUniformF(const GLProgram& prog) const override;
	};
}
