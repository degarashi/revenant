#include "dc/model.hpp"

namespace rev::dc {
	const HTf& IModel::getNode() const noexcept {
		return _tf;
	}
}
