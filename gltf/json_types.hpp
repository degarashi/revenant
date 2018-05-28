#pragma once
#include <cereal/external/rapidjson/document.h>

namespace rev::gltf {
	using JValue = rapidjson::Value;
	using JType = rapidjson::Type;
	using JSize = rapidjson::SizeType;
	using JDocument = rapidjson::Document;
}
