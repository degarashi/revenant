#include "gltf/program.hpp"
#include "gltf/shader.hpp"
#include "../gl_resource.hpp"

namespace rev::gltf {
	using namespace loader;
	Program::Program(const JValue& v):
		Resource(v),
		attribute(Optional<Array<StdString>>(v, "attributes", {})),
		vshader(Required<TagShader>(v, "vertexShader")),
		fshader(Required<TagShader>(v, "fragmentShader"))
	{}
	Resource::Type Program::getType() const noexcept {
		return Type::Program;
	}
	void Program::resolve(const ITagQuery& q) {
		vshader.resolve(q);
		fshader.resolve(q);
	}
	HProg Program::makeProgram() const {
		return mgr_gl.makeProgram(
			vshader.data()->makeShader(),
			fshader.data()->makeShader()
		);
	}
}
