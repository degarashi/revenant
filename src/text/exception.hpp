#pragma once
#include <stdexcept>

namespace rev {
	struct FontNotFound : std::runtime_error {
		using std::runtime_error::runtime_error;
	};
}
