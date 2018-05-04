#pragma once
#include "sys_uniform_if.hpp"
#include "lubee/size.hpp"

namespace rev {
	//! (3D/2D共通)
	/*!
		予め変数名がsys_*** の形で決められていて, 存在すれば計算&設定される
		固定変数リスト:
		vec4 sys_screen;		// x=ScreenSizeX, y=ScreenSizeY, z=1DotSizeX, w=1DotSizeY
	*/
	class SystemUniform : public ISystemUniform {
		private:
			lubee::SizeI	_screenSize;
		public:
			const lubee::SizeI& getScreenSize() const;
			void setScreenSize(const lubee::SizeI& s);
			void outputUniforms(UniformEnt& u) const override;
			void moveFrom(ISystemUniform& prev) override;
	};
}
