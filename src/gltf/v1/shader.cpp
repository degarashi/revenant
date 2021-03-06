#include "gltf/v1/shader.hpp"
#include "gltf/v1/dataquery_if.hpp"
#include "../check.hpp"
#include "../value_loader.hpp"
#include "../../gl/resource.hpp"
#include "../../sdl/rw.hpp"
#include <regex>

namespace rev::gltf::v1 {
	namespace {
		const std::pair<GLenum, ShType> c_type[] = {
			{GL_VERTEX_SHADER, ShType::Vertex},
			{GL_FRAGMENT_SHADER, ShType::Fragment},
		};
	}
	using namespace loader;
	Shader::Shader(const JValue& v, const IDataQuery& q):
		Resource(v),
		src(Required<RWRef>(v, "uri", q))
	{
		const auto t = Required<Integer>(v, "type");
		type = CheckEnum(c_type, t, [](auto&& c, auto&& t){ return c.first==t; }).second;
	}
	Resource::Type Shader::getType() const noexcept {
		return Type::Shader;
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
