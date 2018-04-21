#pragma once
#include <boost/variant.hpp>
#include "idtag.hpp"

namespace rev::draw {
	struct Token;
	using Token_SP = std::shared_ptr<Token>;
}
namespace rev::gltf {
	using UniformValue = boost::variant<
			double, uint8_t, TagTexture,
			std::vector<double>, std::vector<uint8_t>, std::vector<TagTexture>
		>;
	using UniformValueNVP = std::vector<std::pair<GLSLName, UniformValue>>;

	UniformValue LoadUniformValue(const JValue& v);
	draw::Token_SP MakeUniformToken(const UniformValue& value, GLenum type, std::size_t count);
}
