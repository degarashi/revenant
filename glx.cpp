#include "glx.hpp"
#include "gl_error.hpp"
#include "gl_program.hpp"
#include "glx_block.hpp"
#include <boost/format.hpp>

namespace rev {
	GLEffect::EC_FileNotFound::EC_FileNotFound(const std::string& fPath):
		EC_Base((boost::format("file path: \"%1%\" was not found.") % fPath).str())
	{}
	// ----------------- GLEffect -----------------
	GLEffect::GLEffect(const std::string& name):
		_tech(MakeGLXMaterial(parse::LoadGLXStructSet(name)))
	{
		_setConstantUniformList(&GlxId::GetUnifList());
		_setConstantTechPassList(&GlxId::GetTechList());
	}
}
