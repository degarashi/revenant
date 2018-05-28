#pragma once
#include <cereal/external/rapidjson/document.h>

namespace rev::gltf::v1 {
	using JValue = rapidjson::Value;
	using JType = rapidjson::Type;
	using JsonKey = const char*;
	using JDocument = rapidjson::Document;
}
