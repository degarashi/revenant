#include "gltf/program.hpp"
#include "gltf/shader.hpp"
#include "../gl_resource.hpp"

namespace rev::gltf {
	using namespace loader;
	Program::Program(const JValue& v):
		Resource(v),
		vshader(Required<TagShader>(v, "vertexShader")),
		fshader(Required<TagShader>(v, "fragmentShader"))
	{
		auto attr = Optional<Array<StdString>>(v, "attributes", {});
		const auto len = attr.size();
		attribute.resize(len);
		for(std::size_t i=0 ; i<len ; i++) {
			attribute[i] = std::move(attr[i]);
		}
	}
	Resource::Type Program::getType() const noexcept {
		return Type::Program;
	}
	void Program::resolve(const ITagQuery& q) {
		vshader.resolve(q);
		fshader.resolve(q);
	}
	const HProg& Program::makeProgram() const {
		if(!cache) {
			cache = mgr_gl.makeProgram(
				vshader.data()->makeShader(),
				fshader.data()->makeShader()
			);
		}
		return cache;
	}
}
