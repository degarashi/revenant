#pragma once
#include <boost/variant.hpp>
#include "dataref.hpp"

namespace rev {
	class UniformEnt;
}
namespace rev::gltf::v1 {
	using UniformValue = boost::variant<
			double, uint8_t, DRef_Texture,
			std::vector<double>, std::vector<uint8_t>, std::vector<DRef_Texture>
		>;
	using UniformValueNVP = std::vector<std::pair<GLSLName, UniformValue>>;

	UniformValue LoadUniformValue(const JValue& v, const IDataQuery& q);
	void SetUniform(UniformEnt& u, const SName& uname, const UniformValue& value, GLenum type, std::size_t count);
}
