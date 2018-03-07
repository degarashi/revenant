#pragma once
#include "lubee/output.hpp"

namespace rev {
	namespace log {
		struct Entry {
			using Type = lubee::log::Type;
			Type::e		type;
			std::string	msg;

			Entry(const Type::e typ, const std::string& s):
				type(typ),
				msg(s)
			{}
		};
	}
}
