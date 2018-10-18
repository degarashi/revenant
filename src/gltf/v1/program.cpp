#include "gltf/v1/program.hpp"
#include "gltf/v1/shader.hpp"
#include "../../gl_resource.hpp"

namespace rev::gltf::v1 {
	using namespace loader;
	Program::Program(const JValue& v, const IDataQuery& q):
		Resource(v),
		vshader(Required<DRef_Shader>(v, "vertexShader", q)),
		fshader(Required<DRef_Shader>(v, "fragmentShader", q))
	{
		auto attr = OptionalDefault<Array<StdString>>(v, "attributes", {});
		const auto len = attr.size();
		attribute.resize(len);
		for(std::size_t i=0 ; i<len ; i++) {
			attribute[i] = std::move(attr[i]);
		}
	}
	Resource::Type Program::getType() const noexcept {
		return Type::Program;
	}
	const HProg& Program::makeProgram() const {
		if(!cache) {
			cache = mgr_gl.makeProgram(
				vshader->makeShader(),
				fshader->makeShader()
			);
		}
		return cache;
	}
}
