#pragma once
#include "frea/src/quaternion.hpp"
#include "spine/src/enum.hpp"
#include "../../handle/input.hpp"

namespace rev::test {
	class LitRot {
		private:
			frea::Quat		_rot;
		public:
			LitRot();
			frea::Vec3 getDir() const;
			void update(
				const HActC& dir_x,
				const HActC& dir_y,
				const frea::Vec3& right,
				const frea::Vec3& up
			);
	};
	class LitRotObj :
		public LitRot
	{
		private:
			DefineEnum(Act,
				(DirX)(DirY)(LitBtn)
			);
			HAct	_act[Act::_Num];
			bool	_press;
		public:
			LitRotObj();
			void update(
				const frea::Vec3& right,
				const frea::Vec3& up
			);
	};
}
