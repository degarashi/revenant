#pragma once
#include "drawtoken/token.hpp"

namespace rev::draw {
	class BState : public TokenT<BState> {
		private:
			GLenum	id;
			bool	enable;
		public:
			BState(GLenum id, bool enable);
			void exec() override;
	};
}
