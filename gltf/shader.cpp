#include "gltf/shader.hpp"
#include "gltf/check.hpp"
#include "../gl_resource.hpp"
#include "../sdl_rw.hpp"
#include <regex>

namespace rev::gltf {
	namespace {
		const std::pair<GLenum, ShType> c_type[] = {
			{GL_VERTEX_SHADER, ShType::Vertex},
			{GL_FRAGMENT_SHADER, ShType::Fragment},
		};
	}
	using namespace loader;
	Shader::Shader(const JValue& v):
		Resource(v),
		src(Required<TagRW>(v, "uri"))
	{
		const auto t = Required<Integer>(v, "type");
		type = CheckEnum(c_type, t, [](auto&& c, auto&& t){ return c.first==t; }).second;
	}
	Resource::Type Shader::getType() const noexcept {
		return Type::Shader;
	}
	void Shader::resolve(const ITagQuery& q) {
		src.resolve(q);
	}
	namespace {
		const std::regex re_version(R"(^\s*#version)");
	}
	HSh Shader::makeShader() const {
		std::string str = src.getRW()->readAllAsString();
		std::smatch m;
		if(!std::regex_search(str, m, re_version))
			str = "#version 100\n" + str;
		return mgr_gl.makeShader(type, str);
	}
}
