#include "json_parse.hpp"
#include "exceptions.hpp"
#include <cereal/external/rapidjson/error/en.h>

namespace rev::gltf {
	JDocument ParseJSON(const char* json) {
		JDocument doc;
		doc.Parse(json);
		if(doc.HasParseError()) {
			throw SyntaxError(
				json,
				doc.GetErrorOffset(),
				rapidjson::GetParseError_En(doc.GetParseError())
			);
		}
		return doc;
	}
}
