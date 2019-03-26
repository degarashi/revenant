#include "vdecl.hpp"

namespace rev {
	// --------------------------- VDecl for text ---------------------------
	const FWVDecl vertex::text::s_decl(
		VDecl{
			{0, 0, GL_FLOAT, GL_FALSE, 2, {VSemEnum::POSITION, 0}},
			{0, 8, GL_FLOAT, GL_FALSE, 3, {VSemEnum::TEXCOORD, 0}}
		}
	);
}
