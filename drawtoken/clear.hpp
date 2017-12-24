#pragma once
#include "token.hpp"
#include "spine/optional.hpp"
#include "frea/vector.hpp"

namespace rev {
	namespace draw {
		struct ClearParam {
			spi::Optional<frea::Vec4>	color;
			spi::Optional<float>		depth;
			spi::Optional<uint32_t>		stencil;
		};
		class Clear : public TokenT<Clear> {
			private:
				ClearParam	_param;
			public:
				Clear(const ClearParam& p);
				void exec() override;
		};
	}
}
#include "../luaimport.hpp"
DEF_LUAIMPORT(rev::draw::ClearParam)
